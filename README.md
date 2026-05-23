# Black-Scholes Options Greeks Calculator

Live website: https://ShraddhaSharma3.github.io/options-greeks-calculator

A real-time options pricing tool implementing the Black-Scholes model.
Calculates Call/Put prices and Delta, Gamma, Theta, Vega Greeks across
three volatility scenarios for Indian equity markets (NSE).

## Tech
- Website: HTML + JavaScript (zero dependencies)
- Backend logic: C++17 with FTXUI terminal dashboard

## How to Run C++ Version
```bash
mkdir build && cd build
cmake ..
make -j4
./greeks
```
