# Payment Card Validation & Transaction Risk Engine

An educational C++17 payment transaction project demonstrating:

- Card network identification
- Luhn validation
- Expiry and CVV validation
- Rule-based risk scoring
- APPROVED / DECLINED decisions
- C++ thread pool with 4 worker threads
- Thread-safe PostgreSQL persistence
- Transaction history
- CMake + Ninja
- PostgreSQL

> Educational project only. Never use real card numbers, CVVs, secrets, or production payment data.

## Architecture

```text
User / CLI
    |
    v
TransactionProcessor
    |
    +--> CardValidator
    |      +--> Network detection
    |      +--> Luhn
    |      +--> Expiry
    |      +--> CVV format
    |
    +--> RiskEngine
    |      +--> Amount rule
    |      +--> Velocity rule
    |      +--> International rule
    |
    +--> TransactionStore
           |
           v
       PostgreSQL
```

## Risk rules

- Amount > ₹100,000: +30
- 5 or more transactions for the same token in 5 minutes: +20
- International transaction: +15
- Risk 0-30: LOW / APPROVED
- Risk 31-60: MEDIUM / APPROVED
- Risk 61+: HIGH / DECLINED

Invalid card details are rejected before authorization.

## Windows: MSYS2 UCRT64

### 1. Install dependencies

Open **MSYS2 UCRT64** and run:

```bash
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-postgresql
```

Verify:

```bash
g++ --version
g++ -dumpmachine
```

The architecture should be:

```text
x86_64-w64-mingw32
```

### 2. PostgreSQL

Create a database called:

```text
payment_risk_db
```

Then execute `sql/schema.sql` in that database.

The application currently uses:

```text
host=localhost
port=5432
dbname=payment_risk_db
user=postgres
password=postgres
```

If your password is different, edit `src/main.cpp` locally. Do NOT commit a real password to GitHub.

### 3. Configure

From the project root:

```bash
rm -rf build
cmake -S . -B build -G Ninja
```

### 4. Build

```bash
cmake --build build
```

### 5. Run

For PostgreSQL's runtime DLL:

```bash
export PATH="/c/Program Files/PostgreSQL/18/bin:$PATH"
```

Then:

```bash
./build/payment-risk-engine.exe
```

## Test transaction

Use only test data. A common Luhn test Visa number is:

```text
4111111111111111
```

Example:

```text
Card number: 4111111111111111
Expiry: 1227
CVV: 123
Amount: 25000
International: n
```

Then choose:

```text
2. View Transaction History
```

The transaction should remain in PostgreSQL after the application exits.

Verify in psql:

```sql
SELECT transaction_id, network, amount, risk_score,
       risk_level, decision, reason, created_at
FROM transactions
ORDER BY created_at DESC;
```

## Important security note

This is an interview/learning project. It deliberately does not store the PAN or CVV in PostgreSQL. The application creates an educational token from the input card number. This is **not production-grade tokenization**.

A production payment system requires PCI-DSS controls, secure tokenization/HSMs, encrypted secrets, authentication/authorization, audit logging, monitoring, rate limiting, secure APIs and proper database connection management.

## Resume description

**Payment Card Validation & Transaction Risk Engine**  
`C++17 | Multithreading | PostgreSQL | CMake | Linux/MSYS2`

- Developed a C++17 payment transaction engine with card-network identification, Luhn validation, expiry/CVV checks and deterministic risk scoring.
- Implemented a 4-worker thread-pool architecture and thread-safe PostgreSQL transaction persistence.
- Designed rule-based authorization using transaction amount, velocity and international-transaction signals with LOW/MEDIUM/HIGH risk classification.
- Built an interactive CLI for transaction processing and history retrieval while avoiding persistence of raw card numbers and CVVs.
