#ifndef ATM__CONTROLLER_IMPL_HPP_
#define ATM__CONTROLLER_IMPL_HPP_
#include <cstdint>
#include <string>
#include "atm/bank.hpp"
#include "atm/controller.hpp"
#include "atm/driver.hpp"

namespace atm {


class ControllerServiceImpl;

class DriverStateListenerImpl : public IDriverStateListener {
public:
  explicit DriverStateListenerImpl(ControllerServiceImpl* _service);

  void onCardInserted(const std::string& cardNumber) override;
  void onCardEjected() override;
private:
  ControllerServiceImpl* service;
};

class IBlocker {
public:
  virtual ~IBlocker() {}
  virtual void block(int32_t timeMs) = 0;
};

class Blocker : public IBlocker {
public:
  void block(int32_t timeMs) override;
};

class ControllerServiceImpl : public IControllerService {
public:
  explicit ControllerServiceImpl(std::shared_ptr<IBankService> _bankService,
                                 std::shared_ptr<IDriverService> _driverService,
                                 std::shared_ptr<IBlocker>);

  // IControllerService
  std::variant<bool, ControllerError> authenticate(const Card& card, int32_t pinNumber) override;
  std::variant<AccountInfo, ControllerError> requestAccountInfo(const Card& card) override;
  std::variant<int32_t, ControllerError> requestDeposit() override;
  std::variant<AccountInfo, ControllerError> confirmDeposit(const Card& card, int32_t deposit) override;
  std::variant<AccountInfo, ControllerError> requestWithdraw(const Card& card, int32_t withdraw) override;
  void registerControllerStateListener(std::shared_ptr<IControllerStateListener> listener) override;

protected:
  std::variant<std::monostate, ControllerError> returnTakenMoney();
  bool authenticated();
  void clearAuthentication();
  void authenticationExpired();
  bool ensureReadyState();

private:
  std::shared_ptr<IBankService> bankService;
  std::shared_ptr<IDriverService> driverService;
  std::shared_ptr<IBlocker> blocker;
  std::shared_ptr<IControllerStateListener> controllerStateListener;
  std::shared_ptr<IDriverStateListener> driverStateListener;

  std::string token;
  int32_t takenMoney;

  friend class DriverStateListenerImpl;
};

} // namespace atm

#endif  // ATM__CONTROLLER_IMPL_HPP_