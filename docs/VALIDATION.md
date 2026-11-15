# Validação desta entrega

Ambiente: Debian 13, x86-64, GCC 14.2, CMake 4.4.5, JUCE 8.0.6 (`51a8a6d7aeae7326956d747737ccf1575e61e209`).

| Verificação | Resultado |
| --- | --- |
| Build Release Linux de VST3 e Standalone | Passou |
| `VocalDSP` via CTest | Passou: 18 modos, 44,1/48/96 kHz, 62 presets, bypass, ganho, shelves, compressão, delay e tamanhos de bloco |
| Delay sincronizado | Passou: Eight/Quarter em 60/120/180 BPM |
| `VocalProcessor` via CTest | Passou: 14 parâmetros, 62 programas mapeados integralmente, estado, dados inválidos, mono→mono, mono→estéreo, estéreo→estéreo, blocos de 0/1/127/512/4096 amostras |
| DSP com AddressSanitizer + UndefinedBehaviorSanitizer | Passou, sem falhas detectadas |
| Editor JUCE | Construído e renderizado para PNG em modo headless; inspeção visual realizada |
| Imagem de referência | SHA-256 idêntico ao original salvo |
| Repetição do script de ambiente | Passou, usando dependências e artefatos existentes |
| Build Windows x64 com MSVC | **Não executado** nesta máquina Linux; preset e workflow preparados |
| Tentativa Windows via MinGW | **Falhou por incompatibilidade explícita do JUCE 8**, que rejeita MinGW. Não foi gerado binário Windows |
| Reprodução em DAW/hardware Windows | **Não executada** |
| Equivalência sonora com Waves CLA Vocals | **Não estabelecida**; o DSP é próprio |

As duas suítes CTest passaram. A suíte DSP contém 6.362.711 verificações numéricas, muitas delas verificações de cada amostra; isso não representa milhões de casos independentes. Os presets foram testados numericamente, mas ainda precisam de avaliação auditiva com gravações reais.

## Reproduzir neste ambiente

```sh
cd /workspace/CLA-VOCALS
bash tools/cloud-build.sh
mkdir -p /workspace/deps/cache
XDG_CACHE_HOME=/workspace/deps/cache \
  ./build/linux/VocalProcessorTests_artefacts/Release/VocalProcessorTests \
  /workspace/scratch/vocal-ui.png
```

O script configura dependências de desenvolvimento sem root em `/workspace/deps`, preserva a verificação TLS e usa listas autenticadas do Debian. Não modifica os arquivos-fonte, a arte nem os lockfiles durante o build. O JUCE fica fixado pelo hash do commit. Os processos não precisam ficar ativos para desenvolver/testar o plugin.

## Imagem preservada

`Assets/cla-vocals-reference.png` e `/workspace/assets/cla-vocals-reference.png`:

```text
9f520d02a8cf261ad534c060ed965c1ae621dde57f87d9041ed8dc9861e2f606
```

O PNG não foi retocado, recortado nem recomprimido. Na interface em execução, os controles e medidores móveis são desenhados sobre o fundo; por isso uma captura do plugin pode apresentar posições/valores diferentes da imagem estática.

## Próximo passo para Windows

Executar o preset `windows-x64` com Visual Studio 2022 ou o workflow de GitHub Actions incluído. Em seguida, instalar o bundle `.vst3` gerado e testar a abertura, áudio, automação, presets e restauração de sessão em uma DAW Windows 64 bits. Não publicar como release validada antes dessa verificação.

Os campos `install_script` e `start_skill` foram salvos no rascunho das configurações do ambiente. Isso não publica o ambiente. Para reutilizar o snapshot em novas tarefas, revisar/salvar nas configurações do ambiente e publicar pelo produto.
