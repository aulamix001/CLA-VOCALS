# Implementação e correspondência do manual

O manual enviado descreve faders −10..+10, controles de cores, bypass, entrada, saída e medidores. Esta implementação preserva essas famílias de controles e seus nomes. A interface usa o PNG original como recurso binário, sem requisições de rede ou alteração no arquivo.

| Controle | Implementação própria |
| --- | --- |
| Input/Output | Ganho linear convertido de dB, suavizado em 20 ms |
| Bass | Low shelf RBJ, 80/180/350 Hz, ganho −10..+10 dB |
| Treble | High shelf RBJ, 2,5/6/10 kHz, ganho −10..+10 dB |
| Push/Spank/Wall | Compressor estéreo com detector de pico vinculado, razões 2:1/4:1/10:1, ataques 20/6/1 ms e releases 200/90/45 ms |
| Tight/Large/Chamber | Quatro combs amortecidos por canal, tempos diferentes por canal, decaimentos aproximados 0,6/1,7/2,8 s; soma ao sinal direto |
| Slap/Eight/Quarter | Delay fracionário, feedback 0,32, 120 ms / meia batida / uma batida; diferença de 1,3% entre canais |
| Stereo/Wide/Spreader | Duas janelas de delay com crossfade, detune de aproximadamente ±5/10/15 cents |

Esses números são decisões de engenharia deste projeto e não especificações confirmadas da Waves. Reverb, delay e pitch chegam a 50% de sinal somado, mantendo o caminho seco. O fader Compress determina limiar (−8 a −32 dB) e compensação (0 a 6 dB), com bypass suavizado. Não há lookahead nem latência global; delays existem apenas nas ramificações de efeitos. Buffers são alocados em `prepareToPlay`; o callback não aloca memória.

O modo padrão é bypass para cada módulo e os faders iniciam em zero. Como as imagens do manual têm baixa resolução, detalhes não legíveis de defaults e temporizações não foram presumidos como garantias. O banco de presets é autoral, sem extração de presets comerciais. Selecionar um programa substitui todos os 14 valores, evitando carregar modos de um preset anterior.

O original é um produto da Waves. O nome do binário deste projeto é Vocal Suite e sua identidade VST3 é própria, para coexistir com plugins de terceiros sem colisão de IDs.
