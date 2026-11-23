#pragma once

#include <array>

class ReglesDosJugadors
{
public:
    enum class Resultat { Cap, Continua, CanviTorn, Falta, Victoria, Derrota, RepetirTrencada };

    void reinicia()
    {
        *this = ReglesDosJugadors();
    }

    bool potTirar() const
    {
        return !enCurs_ && guanyador_ == 0;
    }

    bool iniciaTir()
    {
        if (!potTirar()) return false;
        enCurs_ = true;
        blancaEnMa_ = false;
        primerContacte_ = 0;
        primeraColor_ = 0;
        bandaDespresContacte_ = false;
        embocadesTir_.fill(false);
        return true;
    }

    void registraContacte(int bola)
    {
        if (enCurs_ && primerContacte_ == 0 && bola >= 1 && bola <= 15)
            primerContacte_ = bola;
    }

    void registraBanda()
    {
        if (enCurs_ && primerContacte_ != 0)
            bandaDespresContacte_ = true;
    }

    void registraEmbocada(int bola)
    {
        if (!enCurs_ || bola < 0 || bola > 15 || embocadesTir_[bola]) return;
        embocadesTir_[bola] = true;
        if (bola > 0)
        {
            embocades_[bola] = true;
            if (bola != 8 && primeraColor_ == 0)
                primeraColor_ = bola;
        }
    }

    Resultat finalitzaTir()
    {
        if (!enCurs_) return Resultat::Cap;
        enCurs_ = false;
        const int jugadorAbans = jugador_;
        const int grupActual = grup_[jugadorAbans - 1];
        const int restantsAbans = comptaRestants(grupActual) + comptaEmbocadesAquestTir(grupActual);
        bool hiHaEmbocades = false;
        for (int bola = 1; bola <= 15; ++bola)
            if (embocadesTir_[bola]) hiHaEmbocades = true;

        if (trencada_ && embocadesTir_[8])
            return Resultat::RepetirTrencada;

        bool contacteIncorrecte = primerContacte_ == 0;
        if (grupActual == 0)
            contacteIncorrecte = contacteIncorrecte || primerContacte_ == 8;
        else if (restantsAbans == 0)
            contacteIncorrecte = contacteIncorrecte || primerContacte_ != 8;
        else
            contacteIncorrecte = contacteIncorrecte || !esDelGrup(primerContacte_, grupActual);

        const bool falta = embocadesTir_[0] || contacteIncorrecte || (!hiHaEmbocades && !bandaDespresContacte_);

        if (embocadesTir_[8])
        {
            const bool victoriaLegal = !falta && !trencada_ && grupActual != 0 && restantsAbans == 0;
            guanyador_ = victoriaLegal ? jugadorAbans : 3 - jugadorAbans;
            return victoriaLegal ? Resultat::Victoria : Resultat::Derrota;
        }

        if (!trencada_ && !falta && grupActual == 0 && primeraColor_ != 0)
        {
            const int assignat = primeraColor_ <= 7 ? 1 : 2;
            grup_[jugadorAbans - 1] = assignat;
            grup_[2 - jugadorAbans] = 3 - assignat;
        }

        trencada_ = false;
        if (falta)
        {
            jugador_ = 3 - jugador_;
            blancaEnMa_ = true;
            tirsDisponibles_ = 2;
            return Resultat::Falta;
        }

        const int grupDespres = grup_[jugadorAbans - 1];
        bool embocadaValida = false;
        if (grupDespres == 0)
            embocadaValida = hiHaEmbocades;
        else
            for (int bola = 1; bola <= 15; ++bola)
                if (embocadesTir_[bola] && esDelGrup(bola, grupDespres)) embocadaValida = true;

        if (embocadaValida)
            return Resultat::Continua;

        if (tirsDisponibles_ > 1)
        {
            --tirsDisponibles_;
            return Resultat::Continua;
        }

        jugador_ = 3 - jugador_;
        tirsDisponibles_ = 1;
        return Resultat::CanviTorn;
    }

    int jugadorActual() const { return jugador_; }
    int tirsDisponibles() const { return tirsDisponibles_; }
    int guanyador() const { return guanyador_; }
    int grupJugador(int jugador) const { return jugador >= 1 && jugador <= 2 ? grup_[jugador - 1] : 0; }
    bool blancaEnMa() const { return blancaEnMa_; }
    bool tirEnCurs() const { return enCurs_; }
    bool esTrencada() const { return trencada_; }

    int bolesRestants(int jugador) const
    {
        return comptaRestants(grupJugador(jugador));
    }

private:
    bool esDelGrup(int bola, int grup) const
    {
        return (grup == 1 && bola >= 1 && bola <= 7) ||
               (grup == 2 && bola >= 9 && bola <= 15);
    }

    int comptaRestants(int grup) const
    {
        if (grup == 0) return 7;
        int restants = 0;
        for (int bola = 1; bola <= 15; ++bola)
            if (esDelGrup(bola, grup) && !embocades_[bola]) ++restants;
        return restants;
    }

    int comptaEmbocadesAquestTir(int grup) const
    {
        if (grup == 0) return 0;
        int compte = 0;
        for (int bola = 1; bola <= 15; ++bola)
            if (esDelGrup(bola, grup) && embocadesTir_[bola]) ++compte;
        return compte;
    }

    int jugador_ = 1;
    int tirsDisponibles_ = 1;
    int guanyador_ = 0;
    int grup_[2] = { 0, 0 };
    bool trencada_ = true;
    bool enCurs_ = false;
    bool blancaEnMa_ = false;
    int primerContacte_ = 0;
    int primeraColor_ = 0;
    bool bandaDespresContacte_ = false;
    std::array<bool, 16> embocades_{};
    std::array<bool, 16> embocadesTir_{};
};
