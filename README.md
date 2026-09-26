<p align="center">
  <img src="logo.png" alt="Bitcoin Knots Cash" width="160" />
</p>

<h1 align="center">Bitcoin Knots Cash (XBTC)</h1>

<p align="center">
  A BLAKE2b proof-of-work cryptocurrency — a fork of
  <a href="https://bitcoinknots.org">Bitcoin Knots</a> with a fair launch,
  a 21,000,000 coin supply, and decentralized DATUM/TIDES mining.
</p>

---

## What is Bitcoin Knots Cash?

Bitcoin Knots Cash (ticker **XBTC**) is a standalone blockchain based on Bitcoin
Knots v29.4. It keeps Bitcoin's economic rules — 21 million coins, a block every
ten minutes, and a reward that halves every 210,000 blocks — while switching the
proof-of-work algorithm to **BLAKE2b** and the difficulty algorithm to **ASERT
(aserti3-2d)** for smooth, per-block retargeting.

It is designed for **decentralized mining**: miners run their own node and build
their own block templates through a DATUM gateway, so no pool operator controls
what goes into blocks.

## Key parameters

| | |
|---|---|
| **Ticker** | XBTC |
| **Algorithm** | BLAKE2b (from block 1; the genesis block is SHA-256d) |
| **Difficulty** | ASERT (aserti3-2d), 2-day half-life, retargets every block |
| **Block time** | 10 minutes |
| **Total supply** | 21,000,000 XBTC |
| **Halving** | every 210,000 blocks |
| **Initial reward** | 50 XBTC |
| **Coinbase maturity** | 100 blocks |
| **Address format** | bech32 `xbtc1…` |
| **P2P port** | 8555 |
| **Network magic** | `0x58 0x42 0x43 0x21` |
| **Config file** | `bitcoinknotscash.conf` |
| **Data directory** | `~/.bitcoinknotscash` (Linux), `%APPDATA%\BitcoinKnotsCash` (Windows) |

## Download

Pre-built wallets for **Linux** and **Windows** (daemon + Qt GUI) are on the
[Releases](https://github.com/BitcoinKnotsCash/BitcoinKnotsCash/releases) page.

- `bitcoinknotscash-linux64.tar.gz` — Linux x86-64
- `bitcoinknotscash-win64.zip` — Windows x86-64

Each archive contains `bitcoind`, `bitcoin-qt`, `bitcoin-cli`, `bitcoin-tx`, `bitcoin-wallet`.

## Running a node

```sh
./bitcoind -daemon
./bitcoin-cli getblockchaininfo
```

Or launch the GUI wallet (`bitcoin-qt`). A minimal `bitcoinknotscash.conf`:

```ini
server=1
rpcuser=your-rpc-user
rpcpassword=your-strong-password
```

## Mining

Bitcoin Knots Cash is **BLAKE2b** and mined via **DATUM** — miners run their own
node and gateway so they choose their own transactions. See the
[Bitcoin Knots mining guide](https://bitcoinknots.org/learn/mining); the same
approach applies against an XBTC node.

## Building from source

```sh
sudo apt-get install build-essential cmake pkg-config bsdmainutils python3 \
    libevent-dev libboost-dev libsqlite3-dev libzmq3-dev \
    qtbase5-dev qttools5-dev qttools5-dev-tools

cmake -B build -DBUILD_GUI=ON
cmake --build build -j"$(nproc)"
# binaries in build/bin/
```

For reproducible cross-platform builds, use the `depends` system — this is what
the GitHub Actions release workflow uses to produce the Linux and Windows binaries.

## Consensus / launch details

- **Genesis** mined fresh for XBTC; the chain diverges from Bitcoin entirely.
- **BLAKE2b from height 1** — the genesis block is SHA-256d, every block after is
  BLAKE2b. The one-time target shift at the change is folded into the ASERT anchor.
- **ASERT difficulty** anchored at genesis, 2-day half-life, retargets every block.

## License

Released under the MIT license (see [COPYING](COPYING)). A derivative of
[Bitcoin Knots](https://github.com/bitcoinknots/bitcoin) and
[Bitcoin Core](https://github.com/bitcoin/bitcoin) — thanks to their developers.

## Links

- **Source:** https://github.com/BitcoinKnotsCash/BitcoinKnotsCash
- **Issues:** https://github.com/BitcoinKnotsCash/BitcoinKnotsCash/issues
