Vocal Suite para **Windows 64 bits**, formatos **VST3 e Standalone**, C++17/JUCE 8.0.6.

- Seis modulos: Bass, Treble, Compress, Reverb, Delay e Pitch, com tres modos e bypass por modulo.
- 62 presets para vozes femininas, masculinas, backings, bateria, percussao, cordas, teclados e sopros.
- Mapeamento automatico dos 14 parametros ao selecionar um preset, automacao, estado da sessao, Load/Save e A/B.
- Imagem de referencia preservada no codigo-fonte, com controles interativos sobrepostos.

**Download:** baixe `Vocal-Suite-Windows-x64.zip` em Assets, extraia e copie a pasta inteira `Vocal Suite.vst3` para `C:\Program Files\Common Files\VST3\`. Depois, atualize a lista de plugins na sua DAW.

Este ZIP so e publicado apos compilacao MSVC x64, testes de DSP e processador, renderizacao do editor e verificacao do formato PE AMD64 dos binarios no GitHub Actions. A versao ainda precisa de validacao auditiva em DAWs e hardware Windows reais. Binarios sem assinatura digital.

O DSP e os presets sao autorais. A equivalencia sonora com o plugin comercial Waves CLA Vocals nao foi estabelecida. Este projeto nao e afiliado a Waves.

`SHA256SUMS.txt` contem o hash do ZIP; `build-info.json`, dentro do ZIP, registra o commit usado.
