# esphome_mando_movistar_tv

Replicar el mando infrarrojo de MovistarTV usando un ESP32

## Notes

This project is documented in Spanish because Movistar operates in Spain and makes no sense to document it in other language. Please use Google Translator if you want to read it in English.

## Introducción

Hace mucho que tengo contratado MovistarTV, actualmente tengo uno de esos decodificadores UHD que usan un mando infrarrojo y quería ver si podía usarlo de alguna manera desde Home Assistant.

A día de hoy (Diciembre del 2022) la plataforma más integrada con Home Assistant para realizar dispositivos DIY es ESPHome, y, ESPHome dispone de componentes genéricos para trabajar con emisores y receptores de infrarrojos. A su vez, uno de los chips más versátiles que podemos usar en ESPHome es el ESP32.

Este proyecto ha consistido en hacer una ingeniería inversa de las señales que emite el mando para poder reproducirlas posteriormente.

Algunas personas me han preguntado porqué no he usado un componente ya existente que aprende los códigos de otros mandos; la respuesta es sencilla. Simplemente porque quería disfrutar de la experiencia DIY. Para hacer este proyecto seguramente me habré gastado más de lo que me hubiera gastado en comprar uno de esos cacharros que "aprenden" los códigos de otros mandos y se controlan desde WiFi, pero si hubiera ido por ese camino no hubiera disfrutado de todo lo que he ido haciendo. El objetivo no era sólamente replicar el mando infrarrojo de Movistar TV en un ESP32 sino disfrutar del resolver el reto.

## Instalar y usar el código

Para usar este código necesitarás seguir los siguientes pasos:

1. Copia este repositorio al pc en el que tengas instalado ESPHome y desde el que desarrolles tus aplicaciones. En mi caso lo tengo instalado en local y lo uso desde un contenedor docker. Si tú estás en el mismo caso, deberás descargar este repositorio dentro del docker directamente.
2. Localiza tu carpeta `config` que es donde tienes los archivos `XXX.yaml` con el código de tus dispositivos. Nota, si usas el complemento de Home Assistant para desarrollar tus componentes, la carpeta `config` es la que guarda los archivos `XXX.yaml`. El editor web de ese complemento no te deja copiar archivos sueltos por lo que deberás entrar en el sistema de archivos usando una conexión de terminal.
3. En `config`, crea una sub carpeta llamada `components` si es que no la tienes ya y, en `components` crea otra sub carpeta llamada `remote_base`.
4. Localiza el directorio `esphome` que debe ser hermano de `config` y dentro de `esphone` busca la carpeta `components/remote_base` copia todo el contenido de esa carpeta a `config/components/remote_base`
5. En la carpeta `src` del código que te has descargado hay una carpeta llamada `remote_base` con 3 archivos. Copia ahora estos 3 archivos en la carpeta `config/components/remote_base`. De los 3 archivos que tienes que copiar, los que empiezan por `movistar_*` son nuevos, pero `__init__.py` de la carpeta `src/remote_base` debe remplazar el que ya existía.

Con esto, lo que estamos haciendo es:

1. Copiar a "local" el componente `remote_base` original para poder modificarlo y
2. Añadir el sub componente que proporciona la funcionalidad de replicar el mando de movistar

Los componentes que estén en la carpeta `config/components` remplazan a los que están declarados de forma global.

## Ejemplo de yaml

En el yaml tenemos que añadir:

```yaml
external_components:
  - source: components
```

Esto le dirá al conversor de `yaml` a `c++` que hay componentes `custom` en la carpeta `components` relativo a la ubicación del `yaml`

Luego, definiremos un `remote_transmitter` y le asignaremos una configuración así:

```yaml
remote_transmitter:
  pin: GPIO32
  carrier_duty_percent: 50%
```

No hay nada que comentar, es la configuración estándar.

Por último, podremos usar las nuevas acciones, por ejemplo en un botón así:

```yaml
button:
  - platform: template
    name: "Movistar TV On/Off"
    on_press:
      then:
        - remote_transmitter.transmit_movistar_OnOff
```

## Referencia de acciones que se pueden usar en automatizaciones

- transmit_movistar_OnOff
- transmit_movistar_Rojo
- transmit_movistar_Verde
- transmit_movistar_Amarillo
- transmit_movistar_Azul
- transmit_movistar_Teclado
- transmit_movistar_Persona
- transmit_movistar_Ayuda
- transmit_movistar_SubirVolumen
- transmit_movistar_BajarVolumen
- transmit_movistar_SubirPrograma
- transmit_movistar_BajarPrograma
- transmit_movistar_Guia
- transmit_movistar_Menu
- transmit_movistar_Subir
- transmit_movistar_Bajar
- transmit_movistar_Izquierda
- transmit_movistar_Derecha
- transmit_movistar_OK
- transmit_movistar_Atras
- transmit_movistar_Mute
- transmit_movistar_Favoritos
- transmit_movistar_Retroceder
- transmit_movistar_PlayPause
- transmit_movistar_Avanzar
- transmit_movistar_Parar
- transmit_movistar_Grabar
- transmit_movistar_1
- transmit_movistar_2
- transmit_movistar_3
- transmit_movistar_4
- transmit_movistar_5
- transmit_movistar_6
- transmit_movistar_7
- transmit_movistar_8
- transmit_movistar_9
- transmit_movistar_0
- transmit_movistar_LineaAbajo
- transmit_movistar_Mensajes

## Cosas pendientes

- Verificar los valores de las constantes usando un decodificador infrarrojo a 56.7 Khz (ya pedido)
- Ver la forma de no tener que sobreescribir todo el paquete `remote_base` de ESPHome ya que eso dificulta el mantenimiento

## Cómo funciona el mando de Movistar TV y un poco de historia

El proyecto consistió en dos partes. La primera era capturar y comprender los códigos que emite el mando infrarrojo. La segunda fue la forma de reproducirlos. La verdad es que no ha sido fácil, pero me lo he pasado bien, que era el objetivo.

El emisor de movistarTV emite pulsos infrarrojos cofificados a 57.6Khz para ver este valor usé un simple transistor NPN sensible al infrarrojo, un operacional LM324 para convertir la señal en pulsos de 3.3v, el propio ESP32 y un analizador lógico que compré por unos 7€ y había recibido hace unos días. En esta página <http://zilsel-invent.blogspot.com/2017/04/programmable-autonomous-vehicles.html> encontré los diagramas necesarios para realizar el circuito.

Una vez puestos todos los componentes en la placa de prototipos, grabé la señal que salía desde el mando en el analizador lógico. Usé el programa de Logic2 <https://www.saleae.com/downloads/> (que es gratuíto para el propósito que tenía entre manos) y analicé los resultados. Haciendo zoom en las señales recibidas pude comprobar que la frecuencia era de 56.7 Khz

En las pruebas anteriores, usé un receptor infrarrojo tipo TSOP4840 que decodifica las señales de entrada como si la frecuencia fuese de 40 Khz, el resultado no es estable, pero es interpretable. He pedido un receptor infrarrojo a 56.7 Khz para ver si con él tengo resultados más estables. La comprobación de que la frecuencia es de 56.7 Khz la tuve la primera vez que intenté enviarle los códigos capturados al decodificador. El decodificador no reaccionaba a ninguna de las frecuencias anteriores, pero cuando lo configuré a 56.7 Khz empezó a reaccionar.

El circuito emisor es muy simple. Consiste en un transistor NPN que se excita en la base con un pin del ESP32 (configurado como el pin de salida de pulsos infrarrojos) en el que el colector se alimenta a 5v para que el led infrarrojo emisor tenga más fuerza. La resistencia de la base es de 10KΩ, la resistencia del led es de 270Ω

Una vez realizado el circuito de entrada, pasé a analizarlos para ver en qué consistían. Encontré información sobre el formato "Pronto" en esta web <https://www.remotecentral.com/features/irdisp1.htm> hay 4 páginas de documentación que son muy explicativas sobre cómo funcionan los mandos remotos por infrarrojos.

Lamentáblemente, los códigos que salían del formato Pronto (con el decodificador TSOP4840) eran más o menos así:

0000 006D 000F 0000 0025 0024 0010 0091 000C 0025 000D 0025 000D 0025 000D 0025 000D 0025 000D 0025 000D 003D 000D 000C 000D 0025 000D 0025 000D 003D 000D 0018 000D 06C3

y no eran concluyentes para identificar patrones ya que no se podía ver qué era un 1 y qué era un 0 por lo que tuve que usar un camino alternativo. Usé el decodificador tipo RAW que lo que hace es sacar números que indican la duración de los pulsos altos y bajos. Usé uno de los botones para explorar esas duraciones; pongo aquí un ejemplo:

```txt
Received Raw: Mantener
1006, -899,
699, -1213,
369,                   -588,
368,   -935,
344,   -907,
368,   -935,
340,   -911,
368,   -907,
368,        -1217,
369,   -919,
343,        -1214,
392,                   -552,
392,                   -564,
368,   -935,
344,        -1867,
393,   -878,
372

Received Raw: Soltar
1006, -927,
670, -1213,
369,                   -616,
340,   -910,
368,       -1536,
369,             -289,
348,   -906,
369,   -910,
368,       -1213,
369,   -894,
368,       -1213,
368,                   -576,
368,                   -617,
340,   -935,
339,       -1218,
368,       -1540,
368
```

como se puede ver, clasifiqué las duraciones en columnas para ver cuales podían ser más o menos similares y, a partir de estos valores, identificar los códigos. Todo esto lo tendré que repetir con el decodificador de 57.6 Khz y espero entonces ver números menos variables.

Al final, después de analizar todos los botones, llegué a la conclusión de que existen seis códigos identificables claramente para los valores negativos y tres para los valores positivos. Con ésto, concluí en usar un sistema de codificación no binario (usé base 6) y pasé a identificar las partes comunes de cada botón.

Las pulsaciones de los botones del mando generan una cabecera que es siempre la misma hasta un punto. En ese punto existen dos posibilidades. Una para codificar cuando el botón se pulsa y otra cuando se suelta. El patrón de pulsar y el de soltar es también reconocible, se repite siempre, en todos los casos, por los que opté por sacarlos fuera de la codificación y meterlos, de forma directa, en la implementación del patrón de reconocimiento de entrada y del reconocimiento de salida (ver cómo funciona `decode` y `encode` en el código fuente). De esa forma, cada tecla tiene solo las partes únicas que le corresponden y la identifican.

Definí en unas contantes los tiempos medios de los pulsos las cuales pongo a continuación:

```c++
static const uint32_t HEADER_HIGH_US = 950;
static const uint32_t HEADER_LOW_US = 950;
static const uint32_t HEADER2_HIGH_US = 647;
static const uint32_t HEADER2_LOW_US = 1265;
static const uint32_t BIT_HIGH_US = 318;
static const uint32_t BIT_HIGH2_US = 648;
static const uint32_t BIT_1_MARK = 1585;
static const uint32_t BIT_0_MARK = 959;
static const uint32_t BIT_A_MARK = 639;
static const uint32_t BIT_B_MARK = 318;
static const uint32_t BIT_Y_MARK = 380;
static const uint32_t BIT_Z_MARK = 624;
```

Una vez identificados los patrones anteriores y asignados los tiempos, capturé la codificación de los botones en la siguiente tabla:

| Tecla         | Pulsar    | Soltar    |
|---------------|-----------|-----------|
| OnOff         | 001B001AY | 001B0001Y |
| Rojo          | 1A11D00Y  | 1A11D1BY  |
| Verde         | 0100D01Y  | 0100D11Y  |
| Amarillo      | 11AB0000Y | 11AB001BY |
| Azul          | 0110D01Y  | 0110D1AY  |
| Teclado       | 0101D1AY  | 0101D11Y  |
| Persona       | 1A1AA01AY | 1A1AA011Y |
| Ayuda         | 110D01AY  | 110D01Z   |
| SubirVolumen  | 1A1BA001Y | 1A1BA010Y |
| BajarVolumen  | 010BA001Y | 010BA01AY |
| SubirPrograma | 01AAA01BY | 01AAA010Y |
| BajarPrograma | 110AB011Y | 110AB01AY |
| Guia          | 1100B001Y | 1100B01AY |
| Menu          | 0110D01Y  | 0110D10Y  |
| Subir         | 01AB001Z  | 01AB001AY |
| Bajar         | 1A1B001BY | 1A1B0010Y |
| Izquierda     | 101B001AY | 101B0011Y |
| Derecha       | 110B0010Y | 110B0011Y |
| OK            | 0010D00Y  | 0010D1BY  |
| Atras         | 1011D01Y  | 1011D1AY  |
| Mute          | 1101D01Y  | 1101D10Y  |
| Favoritos     | 011AA011Y | 011AA001Y |
| Retroceder    | 0100B01BY | 0100B010Y |
| PlayPause     | 0110B01Z  | 0110B01AY |
| Avanzar       | 01A0B01AY | 01A0B011Y |
| Parar         | 1A10B010Y | 1A10B011Y |
| Grabar        | 01B1A01AY | 01B1A01Z  |
| 1             | 101AA010Y | 101AA011Y |
| 2             | 110AA000Y | 110AA01BY |
| 3             | 11AAA001Y | 11AAA01AY |
| 4             | 011D01AY  | 011D011Y  |
| 5             | 011D010Y  | 011D011Y  |
| 6             | 010D000Y  | 010D01BY  |
| 7             | 1A1D001Y  | 1A1D01AY  |
| 8             | 101D001Y  | 101D010Y  |
| 9             | 111D001Y  | 111D011Y  |
| 0             | 011BA010Y | 011BA011Y |
| LineaAbajo    | 001BA01AY | 001BA011Y |
| Mensajes      | 101BA001Y | 101BA011Y |

Ahora sólo quedaba codificar y comprobar que funcionaba la decodificación inversa. En realidad, esta es la fase que más me costó. El motivo era que el camino inicial elegido de usar un TSOP4840 no era el correcto ya que al codificar de nuevo los mensajes de salida usando esa frecuencia el decodificador no se enteraba. Llegado a este punto es cuando intenté determinar la frecuencia desde el mando usando un osciloscopio, pero el que tengo no me sirvió (es un modelo muy simple) y probé con el analizador lógico que fué el que realmente me sorprendió ya que capturó los pulsos de hasta 2 microsegundos de duración y pude comprobar que la frecuencia de los mismos era de 56.7 Khz. Cuando usé ese parámetro en el `encoder` el deco comenzó a reaccionar de la misma forma que si estuviera usando el mando original. El proyecto estaba llegando a su fín...

## Agradecimientos

- A mi mujer, por aguantarme
- A mis hijos, por lo mismo que a mi mujer
- Al grupo de telegram <https://t.me/unlocoysutecnologia> al que he consultado algunas cosas, estamos todos un poco locos...
