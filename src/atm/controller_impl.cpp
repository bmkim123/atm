#include <chrono>
#include <thread>
#include "atm/controller.hpp"
#include "controller_impl.hpp"

namespace atm {

constexpr int32_t MONEY_BOX_OPEN_TIME_MILLIS = 5000;

DriverStateListenerImpl::DriverStateListenerImpl(ControllerServiceImpl* _service)
: service(_service)
{
}

std::variant<Card, ControllerError> parseCardNumber(const std::string& cardNumber) {
  if (cardNumber.length() != 16) {
    return ControllerError::CARD_READ_FAILED;
  }
  for (const char& ch : cardNumber) {
    if (ch < '0' || ch > '9') {
      return ControllerError::CARD_READ_FAILED;
    }
  }

  Card card;
  for (int i = 0; i < 4; i++) {
    auto v = cardNumber.substr(i * 4, 4);
    card.numbers[i] = stoi(v);
  }
  return card;
}

void DriverStateListenerImpl::onCardInserted(const std::string& cardNumber)
{
  if (service->controllerStateListener) {
    service->controllerStateListener->onCardInserted(parseCardNumber(cardNumber));
  }
}

void DriverStateListenerImpl::onCardEjected()
{
  if (service->controllerStateListener) {
    service->controllerStateListener->onCardEjected();
    if (service->authenticated()) {
      service->clearAuthentication();
      service->controllerStateListener->onAuthenticationExpired();
    }
  }
}

ControllerServiceImpl::ControllerServiceImpl(std::shared_ptr<IBankService> _bankService,
                                             std::shared_ptr<IDriverService> _driverService,
                                             std::shared_ptr<IBlocker> _blocker)
: bankService(_bankService), driverService(_driverService), blocker(_blocker),
  driverStateListener(std::make_shared<DriverStateListenerImpl>(this))
{
  driverService->registerDriverStateListener(driverStateListener);
}

std::variant<bool, ControllerError> ControllerServiceImpl::authenticate(const Card& card, int32_t pinNumber)
{
  if (authenticated()) {
    clearAuthentication();
  }

  auto ret = bankService->authenticate(card, pinNumber);
  if (const std::string* ptoken = std::get_if<std::string>(&ret)) {
    token = *ptoken;
    return true;
  }

  auto bankError = std::get<BankError>(ret);
  if (bankError == BankError::INVALID_AUTH) {
    return false;
  }
  return ControllerError::UNKNOWN;
}

std::variant<AccountInfo, ControllerError> ControllerServiceImpl::requestAccountInfo(const Card& card)
{
  if (!authenticated()) {
    return ControllerError::NOT_AUTHENTICATED;
  }

  auto ret = bankService->requestAccountInfo(card, token);
  if (const AccountInfo* paccount = std::get_if<AccountInfo>(&ret)) {
    return *paccount;
  }

  auto bankError = std::get<BankError>(ret);
  if (bankError == BankError::INVALID_AUTH) {
    return ControllerError::NOT_AUTHENTICATED;
  }
  return ControllerError::UNKNOWN;
}

std::variant<int32_t, ControllerError> ControllerServiceImpl::requestDeposit()
{
  if (!authenticated()) {
    return ControllerError::NOT_AUTHENTICATED;
  }

  {
    auto ret = driverService->openMoneyBox();
    if (std::get_if<DriverError>(&ret)) {
      return ControllerError::DRIVER_ERROR;
    }
  }

  blocker->block(MONEY_BOX_OPEN_TIME_MILLIS);

  {
    auto ret = driverService->closeMoneyBox();
    if (std::get_if<DriverError>(&ret)) {
      return ControllerError::DRIVER_ERROR;
    }
  }

  {
    auto ret = driverService->takeMoney();
    if (std::get_if<DriverError>(&ret)) {
      // return money to customer
      driverService->openMoneyBox();
      return ControllerError::DRIVER_ERROR;
    }
    takenMoney = std::get<int32_t>(ret);
    return takenMoney;
  }
}

std::variant<AccountInfo, ControllerError> ControllerServiceImpl::confirmDeposit(const Card& card, int32_t deposit)
{
  if (!authenticated()) {
    // return moeny to customer if exists,
    returnTakenMoney();
    return ControllerError::NOT_AUTHENTICATED;
  }

  if (deposit != takenMoney) {
    // user confirmed deposit must be equals to atm driver reported value.
    // return moeny to customer if exists,
    returnTakenMoney();
    return ControllerError::INVALID_REQUEST_ARGS;
  }

  auto ret = bankService->requestDeposit(card, deposit, token);
  if (std::get_if<BankError>(&ret)) {
    returnTakenMoney();
    return ControllerError::BANK_ERROR;
  }

  takenMoney = 0;
  return std::get<AccountInfo>(ret);
}

std::variant<std::monostate, ControllerError> ControllerServiceImpl::returnTakenMoney() {
  if (takenMoney <= 0) {
    return std::monostate{};
  }

  {
    auto ret = driverService->putMoney(takenMoney);
    if (std::get_if<DriverError>(&ret)) {
      return ControllerError::DRIVER_ERROR;
    }
  }

  {
    auto ret = driverService->openMoneyBox();
    if (std::get_if<DriverError>(&ret)) {
      return ControllerError::DRIVER_ERROR;
    }
  }

  blocker->block(MONEY_BOX_OPEN_TIME_MILLIS);

  {
    auto ret = driverService->closeMoneyBox();
    if (std::get_if<DriverError>(&ret)) {
      return ControllerError::DRIVER_ERROR;
    }
  }
  return std::monostate{};
}

std::variant<AccountInfo, ControllerError> ControllerServiceImpl::requestWithdraw(const Card& card, int32_t withdraw)
{
  if (!authenticated()) {
    return ControllerError::NOT_AUTHENTICATED;
  }

  {
    auto ret = bankService->requestWithdraw(card, withdraw, token);
    if (std::get_if<BankError>(&ret)) {
      return ControllerError::BANK_ERROR;
    }
  }

  {
    auto ret = driverService->putMoney(withdraw);
    if (std::get_if<DriverError>(&ret)) {
      return ControllerError::DRIVER_ERROR;
    }
  }

  {
    auto ret = driverService->openMoneyBox();
    if (std::get_if<DriverError>(&ret)) {
      return ControllerError::DRIVER_ERROR;
    }
  }

  blocker->block(MONEY_BOX_OPEN_TIME_MILLIS);

  AccountInfo accountInfo;
  {
    auto ret = bankService->withdrawCompleted(card, withdraw, token);
    if (std::get_if<BankError>(&ret)) {
      return ControllerError::BANK_ERROR;
    }
    accountInfo = std::get<AccountInfo>(ret);
  }

  driverService->closeMoneyBox();
  return accountInfo;
}

void ControllerServiceImpl::registerControllerStateListener(std::shared_ptr<IControllerStateListener> listener)
{
  controllerStateListener = listener;
}

bool ControllerServiceImpl::authenticated()
{
  return !token.empty();
}

void ControllerServiceImpl::clearAuthentication()
{
  token = "";
}

void Blocker::block(int32_t timeMs)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(timeMs));
}

} // namespace atm