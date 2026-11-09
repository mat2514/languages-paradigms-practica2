/* =========================================================================
   Integrantes: Matias Palacio Gomez, Natalia Vergara Mendoza
   =========================================================================
   
   PRACTICE - STARTING CODE (DO NOT CHANGE THE GIVEN SECTIONS)
   Integrated Itinerary and Fare Engine - Medellin Metro System
   Programming Languages and Paradigms (ST0244) - Universidad EAFIT

   Build: g++ -std=c++17 -Wall -Wextra -o practice main.cpp
   Target environment: OnlineGDB, language selector set to C++17

   WHAT YOU RECEIVE
     - the currency formatter, the Leg and ChargedLeg records and the whole
       main() test harness, so that the expected output is reproducible
     - the declarations you must complete, each marked with TODO

   WHAT YOU MUST WRITE
     - PART 1: the TransportMode hierarchy
     - PART 2: the Passenger hierarchy
     - PART 3: chargeItinerary and the Network class
     - PART 4: the four report functions, with STL algorithms only

   The file must compile with ZERO errors and ZERO warnings under
   g++ -std=c++17 -Wall -Wextra. A warning is a defect, not a detail.
   ========================================================================= */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <map>
#include <functional>

/* -------------------------------------------------------------------------
   Utility: Colombian peso formatting (3820 -> "$ 3.820")
   ------------------------------------------------------------------------- */
std::string formatCOP(long long value) {
    std::string digits = std::to_string(value);
    std::string out;
    int count = 0;
    for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
        out.push_back(*it);
        if (++count % 3 == 0 && it + 1 != digits.rend()) {
            out.push_back('.');
        }
    }
    std::reverse(out.begin(), out.end());
    return "$ " + out;
}

/* -------------------------------------------------------------------------
   PART 1. Abstract base class of the transport-mode hierarchy
   ------------------------------------------------------------------------- */
class TransportMode {
public:
    TransportMode(std::string name, int stations)
        : name_(std::move(name)), stations_(stations) {}

    virtual ~TransportMode() = default;          // mandatory: deleted through base pointer

    const std::string& name() const { return name_; }
    int stations() const { return stations_; }

    // la base no puede saber estos dos valores
    virtual long long baseFare() const = 0;
    virtual std::string serviceType() const = 0;

    // valen true salvo que un modo diga lo contrario
    virtual bool allowsStudentFlatFare() const { return true; }
    virtual bool allowsIntegration() const { return true; }
    virtual bool opensIntegrationWindow() const { return true; }

    // el texto se arma una sola vez aqui con lo que da cada subclase
    std::string describe() const {
        return serviceType() + " " + name_ + " (" + std::to_string(stations_) +
               " stations) - full fare " + formatCOP(baseFare());
    }

private:
    std::string name_;
    int stations_;
};

class MetroLine : public TransportMode {
public:
    MetroLine(std::string name, int stations)
        : TransportMode(std::move(name), stations) {}

    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Metro line"; }
};

class TramLine : public TransportMode {
public:
    TramLine(std::string name, int stations)
        : TransportMode(std::move(name), stations) {}

    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Tram line"; }
};

class CableLine : public TransportMode {
public:
    CableLine(std::string name, int stations)
        : TransportMode(std::move(name), stations) {}

    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Cable line"; }
};

// reutiliza la tarifa del cable y le suma un recargo
class TouristCable : public CableLine {
public:
    TouristCable(std::string name, int stations, long long surcharge)
        : CableLine(std::move(name), stations), surcharge_(surcharge) {}

    long long baseFare() const override { return CableLine::baseFare() + surcharge_; }
    std::string serviceType() const override { return "Tourist cable"; }

    // sin tarifa de estudiante ni integracion ni ventana
    bool allowsStudentFlatFare() const override { return false; }
    bool allowsIntegration() const override { return false; }
    bool opensIntegrationWindow() const override { return false; }

private:
    long long surcharge_;
};

// gratis porque el troncal ya la cubre
class FeederRoute : public TransportMode {
public:
    FeederRoute(std::string name, int stations)
        : TransportMode(std::move(name), stations) {}

    long long baseFare() const override { return 0; }
    std::string serviceType() const override { return "Feeder route"; }

    // si abriera ventana el resto del viaje saldria gratis
    bool opensIntegrationWindow() const override { return false; }
};

/* -------------------------------------------------------------------------
   PART 2. Passenger hierarchy, Non-Virtual Interface idiom
   fareOn() is NOT virtual: it is the template method that clamps the result.
   computeFare() is the customisation point each profile overrides.
   ------------------------------------------------------------------------- */
// base de los perfiles con el patron nvi
class Passenger {
public:
    explicit Passenger(std::string name) : name_(std::move(name)) {}

    virtual ~Passenger() = default;

    virtual std::string profile() const = 0;

    const std::string& name() const { return name_; }

    // no es virtual para que el recorte a cero se haga una sola vez
    long long fareOn(const TransportMode& mode) const {
        return std::max(0LL, computeFare(mode));
    }

protected:
    // por defecto paga la tarifa plena del modo
    virtual long long computeFare(const TransportMode& mode) const {
        return mode.baseFare();
    }

private:
    std::string name_;
};

// no sobrescribe nada y hereda la tarifa plena
class FrequentPassenger : public Passenger {
public:
    explicit FrequentPassenger(std::string name) : Passenger(std::move(name)) {}

    std::string profile() const override { return "Civica frequent"; }
};

class StudentPassenger : public Passenger {
public:
    explicit StudentPassenger(std::string name) : Passenger(std::move(name)) {}

    std::string profile() const override { return "Municipal student"; }

protected:
    // le pregunta al modo su politica y nunca su clase
    long long computeFare(const TransportMode& mode) const override {
        if (mode.allowsStudentFlatFare()) {
            return 1600;
        }
        return Passenger::computeFare(mode);
    }
};

class SeniorPassenger : public Passenger {
public:
    explicit SeniorPassenger(std::string name) : Passenger(std::move(name)) {}

    std::string profile() const override { return "Senior citizen"; }

protected:
    long long computeFare(const TransportMode& mode) const override {
        return Passenger::computeFare(mode) - 490;
    }
};

class DisabilityPassenger : public Passenger {
public:
    explicit DisabilityPassenger(std::string name) : Passenger(std::move(name)) {}

    std::string profile() const override { return "Person with disability"; }

protected:
    long long computeFare(const TransportMode& mode) const override {
        return Passenger::computeFare(mode) - 1100;
    }
};

class BearerPassenger : public Passenger {
public:
    explicit BearerPassenger(std::string name) : Passenger(std::move(name)) {}

    std::string profile() const override { return "Bearer (occasional)"; }

protected:
    long long computeFare(const TransportMode& mode) const override {
        return Passenger::computeFare(mode) + 580;
    }
};

/* -------------------------------------------------------------------------
   PART 3. The itinerary and the integration rule
   ------------------------------------------------------------------------- */
struct Leg {
    int minute;                   // minutes elapsed since the journey started
    const TransportMode* mode;    // non-owning observer pointer
};

struct ChargedLeg {
    int minute;
    std::string modeName;
    std::string serviceType;
    long long charge;
    bool integrated;              // true: free transfer inside the window
};

// la red es la duena de los modos y main solo los mira
class Network {
public:
    // devuelve una referencia para que main pueda observar el modo guardado
    TransportMode& add(std::unique_ptr<TransportMode> mode) {
        modes_.push_back(std::move(mode));
        return *modes_.back();
    }

    const std::vector<std::unique_ptr<TransportMode>>& modes() const { return modes_; }

    void printInventory() const {
        std::for_each(modes_.begin(), modes_.end(),
            [](const std::unique_ptr<TransportMode>& mode) {
                std::cout << "  - " << mode->describe() << '\n';
            });
    }

private:
    std::vector<std::unique_ptr<TransportMode>> modes_;
};

// cobra el viaje aplicando la regla de integracion
std::vector<ChargedLeg> chargeItinerary(std::vector<Leg> legs,
                                        const Passenger& passenger) {
    const int windowMinutes = 90;
    const int maxFreeTransfers = 2;

    // se ordena una copia porque el itinerario llega desordenado
    std::sort(legs.begin(), legs.end(),
              [](const Leg& a, const Leg& b) { return a.minute < b.minute; });

    std::vector<ChargedLeg> result;
    bool windowOpen = false;
    int windowStart = 0;
    int transfersUsed = 0;

    // unico ciclo del programa
    for (const Leg& leg : legs) {
        const TransportMode& mode = *leg.mode;
        long long charge = 0;
        bool integrated = false;
        const bool windowActive = windowOpen &&
                                  leg.minute - windowStart < windowMinutes &&
                                  transfersUsed < maxFreeTransfers;

        if (!mode.allowsIntegration() || !mode.opensIntegrationWindow()) {
            // paga lo suyo y no toca la ventana
            charge = passenger.fareOn(mode);
        } else if (windowActive) {
            // transbordo gratis
            integrated = true;
            ++transfersUsed;
        } else {
            // paga y abre una ventana nueva
            charge = passenger.fareOn(mode);
            windowOpen = true;
            windowStart = leg.minute;
            transfersUsed = 0;
        }

        result.push_back({leg.minute, mode.name(), mode.serviceType(), charge, integrated});
    }
    return result;
}

/* -------------------------------------------------------------------------
   PART 4. Reports built exclusively with STL algorithms and lambdas
   ------------------------------------------------------------------------- */
long long totalPaid(const std::vector<ChargedLeg>& legs) {
    return std::accumulate(legs.begin(), legs.end(), 0LL,
        [](long long sum, const ChargedLeg& c) { return sum + c.charge; });
}

long long freeTransferCount(const std::vector<ChargedLeg>& legs) {
    return std::count_if(legs.begin(), legs.end(),
        [](const ChargedLeg& c) { return c.integrated; });
}

// agrupa lo pagado por tipo de servicio
std::map<std::string, long long> paidByServiceType(const std::vector<ChargedLeg>& legs) {
    std::map<std::string, long long> totals;
    std::for_each(legs.begin(), legs.end(),
        [&totals](const ChargedLeg& c) { totals[c.serviceType] += c.charge; });
    return totals;
}

// el criterio lo pone quien llama y la red no lo conoce
std::vector<std::string> namesMatching(
        const Network& network,
        const std::function<bool(const TransportMode&)>& policy) {
    std::vector<std::string> names;
    std::for_each(network.modes().begin(), network.modes().end(),
        [&](const std::unique_ptr<TransportMode>& mode) {
            if (policy(*mode)) {
                names.push_back(mode->name());
            }
        });
    return names;
}

// parte del primer elemento para no dejar una coma de mas
std::string joined(const std::vector<std::string>& items) {
    if (items.empty()) {
        return "";
    }
    return std::accumulate(items.begin() + 1, items.end(), items.front(),
        [](const std::string& acc, const std::string& item) {
            return acc + ", " + item;
        });
}

/* -------------------------------------------------------------------------
   MAIN
   ------------------------------------------------------------------------- */
int main() {
    std::cout << "============================================================\n";
    std::cout << "   INTEGRATED ITINERARY AND FARE ENGINE - Valle de Aburra\n";
    std::cout << "============================================================\n\n";

    Network network;
    TransportMode& lineA  = network.add(std::make_unique<MetroLine>("A", 21));
    TransportMode& lineB  = network.add(std::make_unique<MetroLine>("B", 7));
    TransportMode& tramTA = network.add(std::make_unique<TramLine>("T-A Ayacucho", 9));
    TransportMode& cableK = network.add(std::make_unique<CableLine>("K Santo Domingo", 4));
    TransportMode& arvi   = network.add(std::make_unique<TouristCable>("L Arvi", 2, 9180));
    TransportMode& feeder = network.add(std::make_unique<FeederRoute>("130 Robledo", 18));

    std::cout << ">> 1. NETWORK INVENTORY (polymorphic describe)\n";
    network.printInventory();
    std::cout << '\n';

    /* Itinerary deliberately given out of chronological order, so that the
       sort step is observable. */
    const std::vector<Leg> itinerary = {
        {45,  &cableK},
        {0,   &feeder},
        {12,  &lineA},
        {150, &lineB},
        {70,  &tramTA},
        {200, &arvi}
    };

    const FrequentPassenger frequent("Diego");
    const StudentPassenger  student("Valentina");
    const SeniorPassenger   senior("Dona Amparo");

    std::cout << ">> 2. FULL FARE PER PROFILE ON METRO LINE A\n";
    std::cout << "  " << frequent.profile() << " pays " << formatCOP(frequent.fareOn(lineA)) << '\n';
    std::cout << "  " << student.profile()  << " pays " << formatCOP(student.fareOn(lineA))  << '\n';
    std::cout << "  " << senior.profile()   << " pays " << formatCOP(senior.fareOn(lineA))   << '\n';
    std::cout << "  " << student.profile()  << " on the tourist cable pays "
              << formatCOP(student.fareOn(arvi)) << " (no flat fare there)\n\n";

    const std::vector<ChargedLeg> charged = chargeItinerary(itinerary, frequent);

    std::cout << ">> 3. CHARGED ITINERARY (frequent profile)\n";
    std::for_each(charged.begin(), charged.end(), [](const ChargedLeg& c) {
        std::cout << "  min " << c.minute << " | " << c.serviceType << " " << c.modeName
                  << " | " << formatCOP(c.charge)
                  << (c.integrated ? "  [free transfer]" : "") << '\n';
    });
    std::cout << '\n';

    std::cout << ">> 4. REPORTS BUILT WITH STL ALGORITHMS\n";
    std::cout << "  accumulate: total paid -> " << formatCOP(totalPaid(charged)) << '\n';
    std::cout << "  count_if: free transfers -> " << freeTransferCount(charged) << '\n';

    const auto byType = paidByServiceType(charged);
    std::for_each(byType.begin(), byType.end(),
        [](const std::pair<const std::string, long long>& entry) {
            std::cout << "  map: paid on " << entry.first << " -> "
                      << formatCOP(entry.second) << '\n';
        });

    const auto firstPaid = std::find_if(charged.begin(), charged.end(),
        [](const ChargedLeg& c) { return c.charge > 0; });
    std::cout << "  find_if: first leg that actually paid -> "
              << (firstPaid == charged.end() ? std::string("none") : firstPaid->modeName) << '\n';

    const std::function<bool(const TransportMode&)> bigModes =
        [](const TransportMode& m) { return m.stations() > 8; };
    std::cout << "  injected policy: modes with more than 8 stations -> "
              << joined(namesMatching(network, bigModes)) << '\n';

    std::cout << "  total for the student on the same itinerary -> "
              << formatCOP(totalPaid(chargeItinerary(itinerary, student))) << '\n';
    std::cout << "  total for the senior on the same itinerary -> "
              << formatCOP(totalPaid(chargeItinerary(itinerary, senior))) << '\n';

    std::cout << "\n============================================================\n";
    std::cout << "  End of demonstration.\n";
    std::cout << "============================================================\n";
    return 0;
}
