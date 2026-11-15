# Vocal Suite — C++ / JUCE, Windows x64

Plugin de efeitos com controles e modos baseados nas páginas do manual do CLA Vocals fornecidas nesta conversa. Formatos: **VST3 e aplicativo Standalone**, C++17, JUCE **8.0.6**, Windows **64 bits**. Também pode ser compilado em Linux para desenvolvimento e testes.

**Download Windows x64:** [Vocal-Suite-Windows-x64.zip](https://github.com/aulamix001/CLA-VOCALS/releases/download/v0.1.0-build.1/Vocal-Suite-Windows-x64.zip). Extraia e copie a pasta inteira `Vocal Suite.vst3` para `C:\Program Files\Common Files\VST3\`. [Release e checksum](https://github.com/aulamix001/CLA-VOCALS/releases/tag/v0.1.0-build.1). O build MSVC, os testes e a renderização da interface passaram no [GitHub Actions](https://github.com/aulamix001/CLA-VOCALS/actions/runs/43753344140); a avaliação em DAW/hardware reais continua pendente.

O DSP e os **62 presets são originais deste projeto**. O manual descreve a operação, mas não publica os algoritmos, curvas, tempos internos ou respostas impulsivas da Waves. Portanto, este projeto **não é uma reprodução sonora certificada ou bit a bit do CLA Vocals**, nem contém código/binários da Waves. A imagem de referência enviada/gerada é incorporada sem alteração; os elementos interativos são sobrepostos durante a execução. Para distribuição, confira as permissões da arte e marcas e a licença do JUCE (AGPLv3 ou comercial, conforme seu uso).

## Compilar no Windows 64 bits

Instale Visual Studio 2022 com **Desktop development with C++**, Windows SDK, Git e CMake 3.22 ou superior. Em um Developer PowerShell, na raiz deste repositório:

```powershell
cmake --preset windows-x64
cmake --build --preset windows-release --parallel 2
ctest --preset windows-release
```

O CMake baixa o JUCE fixado no commit `51a8a6d7aeae7326956d747737ccf1575e61e209`. Para usar uma cópia existente: acrescente `-DJUCE_DIR=C:/caminho/JUCE` na configuração. O gerador seleciona explicitamente a arquitetura x64.

Resultados:

- `build/windows-x64/VocalSuite_artefacts/Release/VST3/Vocal Suite.vst3` — copie **a pasta VST3 inteira**, não apenas o binário interno, para `C:\Program Files\Common Files\VST3\` e faça a DAW procurar plugins novamente.
- `build/windows-x64/VocalSuite_artefacts/Release/Standalone/Vocal Suite.exe` — selecione o dispositivo de áudio no aplicativo para ouvir a entrada.

O workflow `.github/workflows/windows.yml` compila em Windows, testa e empacota os binários. Pushes em `main` e tags `v*`, ou acionamento manual, publicam uma preview em Releases após os checks passarem; pull requests apenas executam os checks. O pacote usa runtime C++ estático. Não há instalador nem assinatura digital nesta versão.

## Operação

Todos os faders vão de **−10 a +10**, com zero central. Entrada e saída são ganho em dB; Bass e Treble controlam ganho das curvas de equalização. Compress, Reverb, Delay e Pitch são quantidades de efeito em escala própria, **não dB de ganho**. Os faders começam em 0 e os módulos em bypass. Os botões amarelos percorrem **Bypass → verde → azul → vermelho → Bypass**.

| Módulo | Verde | Azul | Vermelho |
| --- | --- | --- | --- |
| Bass | Sub | Lower | Upper |
| Treble | Bite | Top | Roof |
| Compress | Push | Spank | Wall |
| Reverb | Tight | Large | Chamber |
| Delay | Slap | Eight | Quarter |
| Pitch | Stereo | Wide | Spreader |

- Clique no campo superior de presets para escolher categoria e variação; os **14 parâmetros são mapeados automaticamente**, incluindo entrada, saída e bypass/modo de cada módulo. Isso não detecta o instrumento no áudio. Os mesmos programas são disponibilizados ao host; a apresentação depende da DAW.
- Duplo clique em um fader retorna a 0. Shift permite ajuste fino. Todos os parâmetros têm IDs estáveis para automação na DAW.
- Load/Save permitem gravar presets próprios `.vocalpreset`; a sessão da DAW também salva os parâmetros. A/B alterna duas configurações enquanto o editor estiver aberto. As setas superiores desfazem/refazem alterações.
- Medidores indicam pico; clique no medidor para limpar o indicador de clipping. Ajuste Input para evitar picos de 0 dBFS. Não há limitador oculto na saída.
- Delay Slap usa 120 ms; Eight usa meia batida e Quarter uma batida, com BPM informado pela DAW e 120 BPM na ausência de informação. Estes tempos são escolhas deste DSP. Alterações de andamento são suavizadas.
- O bypass da DAW mantém o sinal direto. São suportadas conexões mono→mono, mono→estéreo e estéreo→estéreo. Em mono, os efeitos espaciais são somados a mono.

## Banco de presets

62 pontos de partida em `Source/Presets.h`: Female Vocals (6), Male Vocals (6), Lead Vocals (3), Backing & Choir (5), Speech (3), Drums (10), Percussion (4), Guitars & Bass (7), Strings (3), Keys & Synths (9), Brass & Winds (6).

Inclui voz natural/pop/rock/dance/ballad/rap; backings e coros; podcast/rádio; caixa, bumbo, toms, overheads, room e bateria paralela; pandeiro, shaker, congas e cajón; violão aço/nylon, guitarra base/solo, baixo; violino, cello e naipe; piano, Rhodes, Wurlitzer, synth lead/arpejos, pads, Hammond; sax, trompete, trombone, metais, gaita e acordeon.

Os presets usam entrada 0 dB e compensação de saída fixa. Ajuste ganho para cada gravação. Para bateria paralela, use o preset em um retorno/bus auxiliar e misture esse retorno à bateria direta. Não existe controle de mistura global acrescentado à imagem.

## Testes e limites de validação

```sh
# DSP sem JUCE nem dependências gráficas
cmake -S . -B build/dsp -DVOCAL_DSP_ONLY=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/dsp --parallel 2
ctest --test-dir build/dsp --output-on-failure

# Build completo Linux (requer dependências gráficas/ALSA do JUCE)
cmake -S . -B build/linux -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux --parallel 2
ctest --test-dir build/linux --output-on-failure
```

Os testes exercitam áudio real: bypass, ganho, 18 modos em 44,1/48/96 kHz, resposta de equalização, compressão, tempo do delay, independência do tamanho do bloco e estabilidade dos 62 presets. O teste JUCE verifica parâmetros, mapeamento integral do banco, restauração de estado e roteamento mono/estéreo, incluindo blocos vazios e blocos grandes. Consulte `docs/VALIDATION.md` para os resultados efetivamente obtidos nesta máquina.

Faltam comparação auditiva com o CLA Vocals, validação em DAW Windows, teste com hardware de áudio e avaliação subjetiva dos presets. O projeto não inclui VST2/AAX nem emulação dos algoritmos proprietários. O bypass por módulo suaviza a quantidade; trocar o tipo de curva/sala ainda pode produzir transiente. A detecção é de pico, sem true-peak nem medição LUFS.
