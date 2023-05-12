#ifndef ATM__BANK_HPP_
#define ATM__BANK_HPP_
#include <cstdint>
#include <variant>
#include <string>

namespace atm {

enum class BankError {
  UNKNOWN,
  INVALID_AUTH,
};

struct Card {
  int32_t numbers[4];
};

struct AccountInfo {
  std::string account;
  std::string balance;
};

class IBankService {
public:
  virtual ~IBankService() {}
  virtual std::variant<std::string, BankError> authenticate(const Card& card, int32_t pinNumber) = 0;

  virtual std::variant<AccountInfo, BankError> requestAccountInfo(const Card& card, std::string token) = 0;
  virtual std::variant<AccountInfo, BankError> requestDeposit(const Card& card, int32_t deposit, std::string token) = 0;
  virtual std::variant<std::monostate, BankError> requestWithdraw(const Card& card, int32_t withdraw, std::string token) = 0;
  virtual std::variant<AccountInfo, BankError> withdrawCompleted(const Card& card, int32_t withdraw, std::string token) = 0;
};

} // namespace atm
#endif  // ATM__BANK_HPP_