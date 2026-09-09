# 🎵 Gerador de Música Procedural

> Um gerador musical procedural escrito em **C++20**, capaz de criar composições aleatórias usando teoria musical, gerar melodias, progressões de acordes e linhas de baixo, reproduzir o resultado em tempo real e exportá-lo para MIDI.

---

## ✨ Sobre o projeto

**Gerador de Música Procedural** é um projeto experimental desenvolvido em C++ com o objetivo de explorar a geração algorítmica de música.

Em vez de armazenar músicas pré-definidas, o programa utiliza regras musicais e aleatoriedade controlada para construir novas composições a cada geração.

Cada execução pode produzir uma música diferente.

O projeto atualmente trabalha com:

* 🎼 Escalas musicais
* 🎹 Acordes diatônicos
* 🔀 Progressões de acordes probabilísticas
* 🎵 Geração procedural de melodias
* 🎸 Linhas de baixo
* 🎚️ BPM configurável
* ⏱️ Duração configurável
* 🔊 Reprodução de áudio em tempo real
* 🎹 Exportação para `.mid`
* 📂 Biblioteca de músicas geradas
* 🖥️ Interface via terminal

---

## 🧠 Como funciona

A música é construída em várias etapas.

```text
                 ┌──────────────────┐
                 │  MusicSettings   │
                 │                  │
                 │ BPM              │
                 │ Duração          │
                 │ Tonalidade       │
                 └────────┬─────────┘
                          │
                          ▼
                 ┌──────────────────┐
                 │      Scale       │
                 │                  │
                 │ Major            │
                 │ Minor            │
                 │ Dorian           │
                 │ Phrygian         │
                 │ ...              │
                 └────────┬─────────┘
                          │
              ┌───────────┼───────────┐
              ▼           ▼           ▼
        ┌──────────┐ ┌──────────┐ ┌──────────┐
        │  Chords  │ │  Melody  │ │   Bass   │
        └────┬─────┘ └────┬─────┘ └────┬─────┘
             │            │            │
             └────────────┼────────────┘
                          ▼
                 ┌──────────────────┐
                 │   Composition     │
                 └────────┬─────────┘
                          │
                    ┌─────┴─────┐
                    ▼           ▼
              ┌──────────┐ ┌──────────┐
              │  Audio   │ │   MIDI   │
              │  Player  │ │ Exporter │
              └──────────┘ └────┬─────┘
                                │
                                ▼
                            `.mid`
```

### Progressões harmônicas

As progressões não são completamente aleatórias.

O gerador utiliza **probabilidades de transição entre graus** para produzir progressões que tendem a possuir um comportamento musical mais natural.

Por exemplo:

```text
I → V  35%
I → vi 30%
I → IV 25%
I → ii 10%
```

Enquanto o dominante possui uma forte tendência de resolução:

```text
V → I   80%
V → vi  20%
```

Isso permite que a aleatoriedade exista sem transformar a música em uma sequência completamente caótica.

---

## 🎼 Geração de melodias

A melodia é construída utilizando as notas pertencentes à escala selecionada.

O gerador também tenta evitar movimentos excessivamente desconexos, utilizando a nota anterior como referência para algumas decisões melódicas.

Características atuais:

* Notas pertencentes à escala
* Durações variáveis
* Velocity aleatória
* Movimento conjunto probabilístico
* Diferentes quantidades de notas dependendo da duração da composição

---

## 🎹 Sistema de acordes

O projeto possui suporte para diferentes tipos de acordes:

```cpp
Major
Minor
Diminished
Augmented
Dominant7
Major7
Minor7
```

Cada acorde possui seus próprios intervalos.

Por exemplo:

```text
Major
0 - 4 - 7

Minor
0 - 3 - 7

Diminished
0 - 3 - 6

Dominant7
0 - 4 - 7 - 10
```

Os acordes também carregam o seu grau dentro da escala, permitindo que o gerador de progressões utilize essa informação.

---

## 🔊 Reprodução de áudio

O projeto possui um sintetizador simples baseado em **miniaudio**.

O áudio é gerado diretamente em tempo real a partir das notas da composição.

Atualmente a síntese utiliza:

* Sample rate de 44.1 kHz
* Áudio estéreo
* Onda senoidal
* Envelope de ataque/release
* Controle de velocity
* Controle de volume
* Soft limiting

A arquitetura também possui uma representação intermediária de eventos de áudio:

```text
Composition
     │
     ▼
AudioEvent[]
     │
     ▼
Audio callback
     │
     ▼
miniaudio
     │
     ▼
Sistema de áudio
```

---

## 🎹 Exportação MIDI

As composições podem ser exportadas para arquivos MIDI padrão.

O exporter cria um arquivo **MIDI Format 1** com quatro tracks:

```text
Track 0 → Tempo
Track 1 → Chords
Track 2 → Bass
Track 3 → Melody
```

O projeto utiliza:

```text
480 ticks per quarter note
```

Os arquivos podem ser abertos em praticamente qualquer DAW ou software compatível com MIDI.

Exemplos:

* LMMS
* Ardour
* REAPER
* FL Studio
* Ableton Live
* MuseScore
* outros softwares compatíveis com MIDI

---

## 📁 Estrutura do projeto

```text
GeradorDeMusicaProcedural/
│
├── CMakeLists.txt
├── generated.mid
│
├── libs/
│   ├── libremidi/
│   └── miniaudio.h
│
├── src/
│   │
│   ├── app/
│   │   ├── app.cpp
│   │   └── app.hpp
│   │
│   ├── audio/
│   │   ├── audioEvent.hpp
│   │   ├── player.cpp
│   │   └── player.hpp
│   │
│   ├── composition/
│   │   ├── composition.cpp
│   │   └── composition.hpp
│   │
│   ├── core/
│   │   ├── chord.hpp
│   │   ├── musicSettings.hpp
│   │   ├── note.hpp
│   │   ├── random.hpp
│   │   └── scale.hpp
│   │
│   ├── generators/
│   │   ├── bassGenerator.cpp
│   │   ├── bassGenerator.hpp
│   │   ├── chordProgression.cpp
│   │   ├── chordProgression.hpp
│   │   ├── melodyGenerator.cpp
│   │   └── melodyGenerator.hpp
│   │
│   └── midi/
│       ├── midiExporter.cpp
│       ├── midiExporter.hpp
│       ├── midiPlayer.cpp
│       └── midiPlayer.hpp
│
└── main.cpp
```

---

## 🛠️ Tecnologias

| Tecnologia    | Utilização                    |
| ------------- | ----------------------------- |
| **C++20**     | Linguagem principal           |
| **CMake**     | Build system                  |
| **Ninja**     | Build backend                 |
| **miniaudio** | Reprodução e síntese de áudio |
| **libremidi** | Infraestrutura MIDI           |
| **MIDI**      | Exportação musical            |

---

## 🐧 Compilação

### Requisitos

* C++20 compatible compiler
* CMake
* Ninja
* Sistema de áudio compatível

No Arch Linux:

```bash
sudo pacman -S base-devel cmake ninja
```

### Build

Clone o repositório:

```bash
git clone https://github.com/StressedVibeCoder/Gerador-de-Musica-Procedural
cd Gerador-de-Musica-Procedural
```

Configure:

```bash
cmake -S . -B build -G Ninja
```

Compile:

```bash
cmake --build build
```

Execute:

```bash
./build/GeradorDeMusicaProcedural
```

> O nome do executável pode variar dependendo da configuração do `CMakeLists.txt`.

---

## 🎲 Aleatoriedade

As decisões musicais são baseadas em um gerador pseudoaleatório.

Isso significa que duas gerações com exatamente as mesmas configurações podem produzir resultados diferentes:

```text
Mesma configuração
        │
        ├── Geração #1 → 🎵 Música A
        │
        ├── Geração #2 → 🎵 Música B
        │
        ├── Geração #3 → 🎵 Música C
        │
        └── Geração #4 → 🎵 Música D
```

Isso é intencional.

O objetivo do projeto é explorar como regras simples + aleatoriedade podem gerar resultados musicais complexos.

---

## 🚧 Estado atual

O projeto está em desenvolvimento.

### Implementado

* [x] Modelo de notas
* [x] Escalas
* [x] Acordes
* [x] Acordes diatônicos
* [x] Progressões probabilísticas
* [x] Gerador de melodias
* [x] Gerador de baixo
* [x] Sistema de composição
* [x] Exportação MIDI
* [x] Leitura de MIDI
* [x] Reprodução de MIDI
* [x] Síntese de áudio em tempo real
* [x] Configuração de BPM
* [x] Configuração de duração
* [x] Biblioteca de músicas geradas

### Próximos passos

* [ ] Melhorar a qualidade do sintetizador
* [ ] Instrumentos virtuais diferentes
* [ ] ADSR mais completo
* [ ] Pan estéreo
* [ ] Efeitos de áudio
* [ ] Melhor geração melódica
* [ ] Progressões específicas para diferentes modos
* [ ] Sistema de sementes (`seed`)
* [ ] Reprodução contínua
* [ ] Melhor interface de terminal
* [ ] Mais opções de exportação
* [ ] Testes automatizados
* [ ] Otimização do áudio em tempo real

---

## 🤖 Uso de Inteligência Artificial

Este projeto foi desenvolvido com auxílio de **Inteligência Artificial**.

Partes do código foram **geradas, sugeridas, revisadas ou modificadas com assistência de IA**, incluindo algumas decisões de arquitetura e implementação.

O README deste repositório também foi **elaborado com auxílio de IA**.

A IA foi utilizada como ferramenta de desenvolvimento, mas a integração, testes, decisões finais e evolução do projeto são responsabilidade do autor.

> **Transparência:** nem todo código presente neste repositório foi escrito manualmente pelo autor. Algumas partes foram produzidas ou refinadas com assistência de IA.

---

## 🎯 Objetivo

Este projeto não pretende competir com uma DAW ou com um compositor humano.

O objetivo é experimentar.

A ideia central é explorar até onde é possível chegar combinando:

```text
Teoria Musical
      +
Algoritmos
      +
Probabilidade
      +
Aleatoriedade
      +
Síntese de Áudio
      +
C++
```

e transformar tudo isso em música.

---

## 📜 Licença

Este projeto utiliza componentes de terceiros que possuem suas próprias licenças.

Consulte as respectivas licenças dentro de:

```text
libs/
```

---
## 📜 Licença do Projeto

Copyright (c) 2026 StressedVibeCoder

É concedida, gratuitamente, a qualquer pessoa que obtenha uma cópia
deste software e dos respetivos ficheiros de documentação, a permissão para
utilizar, copiar, modificar, fundir, publicar, distribuir, sublicenciar e/ou
vender cópias do software, bem como permitir que outras pessoas a quem o
software seja fornecido façam o mesmo, desde que sejam respeitadas as
seguintes condições:

O aviso de copyright acima e este aviso de permissão devem ser incluídos em
todas as cópias ou partes substanciais do software.

Este software é fornecido "tal como está", sem qualquer garantia, expressa
ou implícita, incluindo, entre outras, garantias de comercialização,
adequação a uma finalidade específica e não violação. Em nenhuma
circunstância os autores ou detentores dos direitos de autor serão
responsáveis por quaisquer reclamações, danos ou outras responsabilidades,
seja numa ação contratual, extracontratual ou de outra natureza, decorrentes
de, resultantes de ou relacionadas com o software ou com a sua utilização.


---

## ⭐ Contribuições

Issues, sugestões e pull requests são bem-vindos.

Se encontrar um bug, tiver uma ideia para melhorar a geração musical ou quiser experimentar novos algoritmos, fique à vontade para contribuir.

---

<div align="center">

**🎵 Generate. Randomize. Compose.**

Made with C++ and a questionable amount of randomness.

</div>
