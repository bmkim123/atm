#ifndef ATM__CONTROLLER_HPP_
#define ATM__CONTROLLER_HPP_
#include <cstdint>
#include <memory>
#include <variant>
#include "atm/bank.hpp"

namespace atm {

enum class ControllerError {
  UNKNOWN,
  NOT_AUTHENTICATED,
  AUTHENTICATION_FAILED,
  INVALID_REQUEST_ARGS,
  CARD_READ_FAILED,
  DRIVER_ERROR,
  BANK_ERROR,
};


class IControllerStateListener {
public:
  virtual ~IControllerStateListener() {}

  virtual void onCardInserted(std::variant<Card, ControllerError> card) = 0;
  virtual void onCardEjected() = 0;
  virtual void onAuthenticationExpired() = 0;
};

class IControllerService {
public:
  virtual ~IControllerService() {}

  virtual std::variant<bool, ControllerError> authenticate(const Card& card, int32_t pinNumber) = 0;

  virtual std::variant<AccountInfo, ControllerError> requestAccountInfo(const Card& card) = 0;
  virtual std::variant<int32_t, ControllerError> requestDeposit() = 0;
  virtual std::variant<AccountInfo, ControllerError> confirmDeposit(const Card& card, int32_t deposit) = 0;
  virtual std::variant<AccountInfo, ControllerError> requestWithdraw(const Card& card, int32_t widthDraw) = 0;

  virtual void registerControllerStateListener(std::shared_ptr<IControllerStateListener> listener) = 0;
};

} // namespace atm

#endif  // ATM__CONTROLLER_HPP_