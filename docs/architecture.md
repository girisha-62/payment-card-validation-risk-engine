# Architecture

## Request flow

1. CLI collects test card details and transaction attributes.
2. `CardValidator` identifies the network and validates Luhn, expiry and CVV format.
3. `TransactionProcessor` creates a transaction and submits it to the worker pool.
4. `RiskEngine` calculates a deterministic score.
5. `TransactionStore` serializes PostgreSQL access with a mutex.
6. PostgreSQL persists the transaction and card token.
7. The result is printed to the user.

## Concurrency

Four worker threads are created by `ThreadPool`. The CLI waits for each submitted transaction so that the displayed result and transaction history are deterministic.

`TransactionStore` protects its PostgreSQL connection with a mutex because one `PGconn` is not used concurrently by multiple worker operations.

## Data protection

The database schema stores a token rather than a raw PAN and never stores CVV. The token implementation is intentionally educational and is not suitable for production.
