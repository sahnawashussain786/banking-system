# Simple Banking System

A menu-driven console banking application written in standard C. It demonstrates login checks, balance display, deposits, withdrawals, transfers, and account information in one in-memory demo account.

## Build and run

Compile with a C compiler such as GCC or Clang:

```text
gcc -std=c11 -Wall -Wextra -Wpedantic main.c -o banking
```

Run `./banking` on macOS/Linux or `banking.exe` on Windows.

## Demo login

- PIN: `1234`
- Account number: `10001234`
- Starting balance: `25000.00 INR`

Enter the PIN when prompted. The program allows three login attempts. Update the constants near the top of `main.c` to use a different demo PIN or starting balance.

## Operations and input checks

1. Check the current balance.
2. Deposit a positive amount.
3. Withdraw a positive amount when funds are available.
4. Transfer a positive amount when funds are available. The receiver number must contain eight digits and cannot be the current account number.
5. View the account number, status, and current balance.
6. Exit the program.

Amounts are entered in INR with up to two decimal places, such as `125`, `125.5`, or `125.50`. Zero, negative, malformed, and over-precision amounts are rejected. The program stores money as whole paise to avoid floating-point rounding in balance updates.

## Demo scope

This is an educational, single-account simulation. It validates the receiver account number's format but has no recipient directory, so a transfer is recorded as a deduction from the demo account and does not credit another account. Balances reset when the program exits. The hard-coded PIN and visible PIN entry are for demonstration only and are not suitable for real banking.
