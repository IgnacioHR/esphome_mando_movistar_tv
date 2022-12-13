#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/base_automation.h"
#include "remote_base.h"

namespace esphome {
namespace remote_base {

using namespace remote_base;

struct MovistarData {
  bool state;           // true = press, false = release
  std::string command;  // 1, 0, A, B, C, D, E, F, Y, Z

  bool operator==(const MovistarData &rhs) const { return state == rhs.state && command.compare(rhs.command) == 0; }
};

class MovistarProtocol : public RemoteProtocol<MovistarData> {
 public:
  void encode(RemoteTransmitData *dst, const MovistarData &data) override;
  optional<MovistarData> decode(RemoteReceiveData src) override;
  void dump(const MovistarData &data) override;
};

DECLARE_REMOTE_PROTOCOL(Movistar)

template<typename... Ts> class MovistarAction : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(bool, state)
  TEMPLATABLE_VALUE(std::string, command)

  void encode(RemoteTransmitData *dst, Ts... x) override {
    MovistarData data{};
    data.state = this->state_.value(x...);
    data.command = this->command_.value(x...);
    MovistarProtocol().encode(dst, data);
  }
};

template<typename... Ts> class MovistarActionBase : public Action<Ts...> {
 public:
  void set_parent(RemoteTransmitterBase *parent) { this->parent_ = parent; }
  void play(std::string a, std::string b, Ts... x) {
    ActionList<> al;
    MovistarAction<> aa;
    aa.set_parent(this->parent_);
    aa.set_send_times(2);
    aa.set_send_wait(78000);
    aa.set_state(true);
    aa.set_command(a);
    al.add_action(&aa);

    DelayAction<> delayaction;
    delayaction.set_delay(75);
    al.add_action(&delayaction);

    MovistarAction<> ab;
    ab.set_parent(this->parent_);
    ab.set_send_times(1);
    ab.set_send_wait(78000);
    ab.set_state(true);
    ab.set_command(b);
    al.add_action(&ab);

    al.play(x...);
  }

 protected:
  RemoteTransmitterBase *parent_{};
  std::string a;
  std::string b;
};

#define MOVISTAR_ACTION(keyword, press, release) \
  template<typename... Ts> class MovistarAction##keyword : public MovistarActionBase<Ts...> { \
   public: \
    void play(Ts... x) { MovistarActionBase<Ts...>::play(press, release, x...); } \
  };

MOVISTAR_ACTION(OnOff, "001B001AY", "001B0001Y")
MOVISTAR_ACTION(Rojo, "1A11D00Y", "1A11D1BY")
MOVISTAR_ACTION(Verde, "0100D01Y", "0100D11Y")
MOVISTAR_ACTION(Amarillo, "11AB0000Y", "11AB001BY")
MOVISTAR_ACTION(Azul, "0110D01Y", "0110D1AY")
MOVISTAR_ACTION(Teclado, "0101D1AY", "0101D11Y")
MOVISTAR_ACTION(Persona, "1A1AA01AY", "1A1AA011Y")
MOVISTAR_ACTION(Ayuda, "110D01AY", "110D01Z")
MOVISTAR_ACTION(SubirVolumen, "1A1BA001Y", "1A1BA010Y")
MOVISTAR_ACTION(BajarVolumen, "010BA001Y", "010BA01AY")
MOVISTAR_ACTION(SubirPrograma, "01AAA01BY", "01AAA010Y")
MOVISTAR_ACTION(BajarPrograma, "110AB011Y", "110AB01AY")
MOVISTAR_ACTION(Guia, "1100B001Y", "1100B01AY")
MOVISTAR_ACTION(Menu, "0110D01Y", "0110D10Y")
MOVISTAR_ACTION(Subir, "01AB001Z", "01AB001AY")
MOVISTAR_ACTION(Bajar, "1A1B001BY", "1A1B0010Y")
MOVISTAR_ACTION(Izquierda, "101B001AY", "101B0011Y")
MOVISTAR_ACTION(Derecha, "110B0010Y", "110B0011Y")
MOVISTAR_ACTION(OK, "0010D00Y", "0010D1BY")
MOVISTAR_ACTION(Atras, "1011D01Y", "1011D1AY")
MOVISTAR_ACTION(Mute, "1101D01Y", "1101D10Y")
MOVISTAR_ACTION(Favoritos, "011AA011Y", "011AA001Y")
MOVISTAR_ACTION(Retroceder, "0100B01BY", "0100B010Y")
MOVISTAR_ACTION(PlayPause, "0110B01Z", "0110B01AY")
MOVISTAR_ACTION(Avanzar, "01A0B01AY", "01A0B011Y")
MOVISTAR_ACTION(Parar, "1A10B010Y", "1A10B011Y")
MOVISTAR_ACTION(Grabar, "01B1A01AY", "01B1A01Z")
MOVISTAR_ACTION(1, "101AA010Y", "101AA011Y")
MOVISTAR_ACTION(2, "110AA000Y", "110AA01BY")
MOVISTAR_ACTION(3, "11AAA001Y", "11AAA01AY")
MOVISTAR_ACTION(4, "011D01AY", "011D011Y")
MOVISTAR_ACTION(5, "011D010Y", "011D011Y")
MOVISTAR_ACTION(6, "010D000Y", "010D01BY")
MOVISTAR_ACTION(7, "1A1D001Y", "1A1D01AY")
MOVISTAR_ACTION(8, "101D001Y", "101D010Y")
MOVISTAR_ACTION(9, "111D001Y", "111D011Y")
MOVISTAR_ACTION(0, "011BA010Y", "011BA011Y")
MOVISTAR_ACTION(LineaAbajo, "001BA01AY", "001BA011Y")
MOVISTAR_ACTION(Mensajes, "101BA001Y", "101BA011Y")

}  // namespace remote_base
}  // namespace esphome
