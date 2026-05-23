#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

// ============================================================
//  BLACK-SCHOLES MATH
// ============================================================

double normalCDF(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2));
}
double normalPDF(double x) {
    return std::exp(-0.5 * x * x) / std::sqrt(2.0 * M_PI);
}
double calc_d1(double S, double K, double r, double T, double sigma) {
    return (std::log(S / K) + (r + 0.5 * sigma * sigma) * T)
           / (sigma * std::sqrt(T));
}
double calc_d2(double S, double K, double r, double T, double sigma) {
    return calc_d1(S, K, r, T, sigma) - sigma * std::sqrt(T);
}
double callPrice(double S, double K, double r, double T, double sigma) {
    double d1 = calc_d1(S,K,r,T,sigma), d2 = calc_d2(S,K,r,T,sigma);
    return S * normalCDF(d1) - K * std::exp(-r*T) * normalCDF(d2);
}
double putPrice(double S, double K, double r, double T, double sigma) {
    double d1 = calc_d1(S,K,r,T,sigma), d2 = calc_d2(S,K,r,T,sigma);
    return K * std::exp(-r*T) * normalCDF(-d2) - S * normalCDF(-d1);
}
double deltaCall(double S, double K, double r, double T, double sigma) {
    return normalCDF(calc_d1(S,K,r,T,sigma));
}
double deltaPut(double S, double K, double r, double T, double sigma) {
    return normalCDF(calc_d1(S,K,r,T,sigma)) - 1;
}
double gamma_(double S, double K, double r, double T, double sigma) {
    return normalPDF(calc_d1(S,K,r,T,sigma)) / (S * sigma * std::sqrt(T));
}
double thetaCall(double S, double K, double r, double T, double sigma) {
    double d1 = calc_d1(S,K,r,T,sigma), d2 = calc_d2(S,K,r,T,sigma);
    return (-(S * normalPDF(d1) * sigma) / (2 * std::sqrt(T))
            - r * K * std::exp(-r*T) * normalCDF(d2)) / 365;
}
double vega_(double S, double K, double r, double T, double sigma) {
    return S * normalPDF(calc_d1(S,K,r,T,sigma)) * std::sqrt(T) / 100;
}

// ============================================================
//  HELPER: format doubles to string
// ============================================================

std::string fmt(double val, int prec = 4) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(prec) << val;
    return ss.str();
}

// ============================================================
//  MAIN DASHBOARD
// ============================================================

int main() {
    // Default input values
    double S     = 1642.30;  // Stock price
    double K     = 1600.00;  // Strike price
    double r     = 0.065;    // Risk-free rate
    double T_days = 30.0;    // Days to expiry

    // Volatility scenarios
    double vols[3]        = {0.15, 0.22, 0.35};
    std::string volLabels[3] = {"Low (15%)", "Base (22%)", "High (35%)"};
    int selectedVol       = 1;  // default: Base

    // Input fields as strings
    std::string sStr  = "1642.30";
    std::string kStr  = "1600.00";
    std::string rStr  = "0.065";
    std::string tStr  = "30";

    auto screen = ScreenInteractive::TerminalOutput();

    // Input components
    auto inputS = Input(&sStr, "Stock Price");
    auto inputK = Input(&kStr, "Strike Price");
    auto inputR = Input(&rStr, "Risk-Free Rate");
    auto inputT = Input(&tStr, "Days to Expiry");

    
    // Note: FTXUI Toggle takes vector, so:
    std::vector<std::string> volVec = {"Low (15%)", "Base (22%)", "High (35%)"};
    auto toggle = Toggle(&volVec, &selectedVol);

    auto container = Container::Vertical({
        inputS, inputK, inputR, inputT, toggle
    });

    auto renderer = Renderer(container, [&] {
        // Parse inputs safely
        try { S     = std::stod(sStr);  } catch(...) { S = 0; }
        try { K     = std::stod(kStr);  } catch(...) { K = 0; }
        try { r     = std::stod(rStr);  } catch(...) { r = 0; }
        try { T_days = std::stod(tStr); } catch(...) { T_days = 1; }

        double T     = T_days / 365.0;
        double sigma = vols[selectedVol];

        // Calculate results
        double cp = (S > 0 && K > 0 && T > 0) ? callPrice(S,K,r,T,sigma) : 0;
        double pp = (S > 0 && K > 0 && T > 0) ? putPrice(S,K,r,T,sigma)  : 0;
        double dc = (S > 0 && K > 0 && T > 0) ? deltaCall(S,K,r,T,sigma) : 0;
        double dp = (S > 0 && K > 0 && T > 0) ? deltaPut(S,K,r,T,sigma)  : 0;
        double gm = (S > 0 && K > 0 && T > 0) ? gamma_(S,K,r,T,sigma)    : 0;
        double th = (S > 0 && K > 0 && T > 0) ? thetaCall(S,K,r,T,sigma) : 0;
        double vg = (S > 0 && K > 0 && T > 0) ? vega_(S,K,r,T,sigma)     : 0;

        return vbox({
            // Title
            text("  Black-Scholes Options Greeks Calculator  ")
                | bold | color(Color::Cyan) | center,
            separator(),

            // Inputs
            hbox({
                vbox({
                    text(" Stock Price (S) ") | color(Color::Yellow),
                    inputS->Render() | border,
                }) | flex,
                vbox({
                    text(" Strike Price (K) ") | color(Color::Yellow),
                    inputK->Render() | border,
                }) | flex,
                vbox({
                    text(" Risk-Free Rate (r) ") | color(Color::Yellow),
                    inputR->Render() | border,
                }) | flex,
                vbox({
                    text(" Days to Expiry ") | color(Color::Yellow),
                    inputT->Render() | border,
                }) | flex,
            }),

            separator(),

            // Volatility toggle
            hbox({
                text(" Volatility Scenario: ") | color(Color::Yellow),
                toggle->Render(),
            }) | center,

            separator(),

            // Results
            hbox({
                // Prices
                vbox({
                    text(" OPTION PRICES ") | bold | color(Color::Green) | center,
                    separator(),
                    hbox({ text(" Call Price : ") | color(Color::White),
                           text(fmt(cp)) | bold | color(Color::Green) }),
                    hbox({ text(" Put Price  : ") | color(Color::White),
                           text(fmt(pp)) | bold | color(Color::Red) }),
                }) | border | flex,

                // Greeks
                vbox({
                    text(" GREEKS ") | bold | color(Color::Green) | center,
                    separator(),
                    hbox({ text(" Delta (Call) : ") | color(Color::White),
                           text(fmt(dc)) | bold | color(Color::Cyan) }),
                    hbox({ text(" Delta (Put)  : ") | color(Color::White),
                           text(fmt(dp)) | bold | color(Color::Cyan) }),
                    hbox({ text(" Gamma        : ") | color(Color::White),
                           text(fmt(gm)) | bold | color(Color::Yellow) }),
                    hbox({ text(" Theta/day    : ") | color(Color::White),
                           text(fmt(th)) | bold | color(Color::Red) }),
                    hbox({ text(" Vega/1%vol   : ") | color(Color::White),
                           text(fmt(vg)) | bold | color(Color::Magenta) }),
                }) | border | flex,
            }),

            separator(),
            text(" Tab: switch fields | Enter: confirm | q: quit ")
                | color(Color::GrayDark) | center,
        });
    });

    // Quit on 'q'
    auto finalRenderer = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Character('q')) {
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });

    screen.Loop(finalRenderer);
    return 0;
}