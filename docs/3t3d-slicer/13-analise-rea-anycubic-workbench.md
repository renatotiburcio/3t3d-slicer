# Analise REA do Anycubic Slicer Next e plano de consolidacao

## Escopo e identidade dos alvos

O repositorio `https://github.com/develonrails/anycubic-slicer-next` foi clonado
em `analysis/anycubic-slicer-next` para referencia. Ele nao contem o codigo da
interface do slicer: o README descreve um empacotador Flatpak/AppImage que
reempacota o `.deb` oficial da Anycubic.

O executavel instalado em `C:/Program Files/AnycubicSlicerNext/AnycubicSlicerNext.exe`
tem apenas 65.616 bytes e funciona como launcher. A implementacao principal esta
em `AnycubicSlicer.dll` e a Bancada em `Workbench.dll`; portanto a engenharia
reversa de comportamento visual deve priorizar o DLL e os artefatos web
embutidos, nao o launcher.

## Configuracao do REA

O REA foi encontrado instalado na versao 6.5.0. O setup foi aplicado somente
para o cliente Codex, com backup em `C:/Users/renat/.codex/config.toml.rea.backup`.
Tambem foi atualizada a skill local para a identidade REA 42/207 ferramentas.
O Ghidra 12.1.4 ja estava instalado e validado, com Java 21 e decompilador
nativo x64 disponiveis. Hopper, IDA e JADX nao foram instalados porque nao sao
necessarios para PE nativo x64 neste estagio.

A sessao Codex em execucao precisa ser reiniciada para carregar o servidor MCP
recem-registrado. Enquanto isso, o CLI REA foi usado diretamente, sem executar o
Anycubic instalado e sem modificar seus arquivos.

## Evidencia estatica observada

`Workbench.dll` foi analisado com Ghidra via REA.

- SHA-256 observado: `58e083b4619ade85c98aa3d2ec0717e1f2d0f01315f39c6b559662215e32844e`.
- PE x64, aproximadamente 39 MB de `.rdata`, 76 procedimentos e 333 strings
  definidas no inventario.
- A busca de strings encontrou os modulos embarcados:
  `js/src_views_workbench2_WorkbenchV3_vue.js`,
  `js/src_views_printerList_printerList_vue.js`,
  `js/src_views_machineFarm_deviceManagement_vue.js` e
  `js/src_utils_axisControlContext_js`.
- Tambem encontrou assets dedicados como `img/icon-skip...svg`,
  `img/material-edit...svg`, imagens de estado sem impressora e imagens de
  impressora LAN.

Esses fatos mostram que a tela de Bancada da Anycubic e uma aplicacao web Vue
embutida, com componentes de dominio para lista de impressoras, gerenciamento do
dispositivo, controle de eixos, materiais e selecao de objetos. A UI nao e uma
janela wxWidgets equivalente a nossa implementacao atual.

A tentativa de varrer o `AnycubicSlicer.dll` inteiro com uma expressao ampla
atingiu o limite de tempo do Ghidra. Isso e uma limitacao de cobertura, nao uma
conclusao de que o DLL nao contenha informacoes. A proxima analise deve usar
buscas menores e focadas, ou a sessao MCP apos reinicio, preservando cada
Evidence retornada.

## Consequencia para o 3T3D

O dashboard nativo atual foi util para validar o contrato do simulador, mas esta
arquiteturalmente abaixo da referencia porque usa muitos controles wxWidgets
basicos para representar uma composicao web rica. Para equiparar e superar o
Anycubic, a Bancada deve evoluir para uma camada de componentes 3T3D reutilizaveis:

1. `WorkbenchSurface`: superficie, sombra, raio, padding e estados hover/foco.
2. `MetricTile`: icone, titulo, valor, alvo, estado e unidade sem strings
   concatenadas.
3. `JobCard`: miniatura real da tarefa, progresso, camadas, restante, termino
   estimado e acoes contextuais.
4. `PrinterCard`: imagem, status, modelo, serial e selecao de impressora.
5. `MaterialSlot`: rolo, cor real do preset, tipo, slot e estado de secagem.
6. `CameraSurface`: imagem/stream, play, tela cheia e estado indisponivel.
7. `ObjectSkipViewport`: mesma geometria da mesa do modo Preparar, mas com a
   interacao limitada a selecionar, desselecionar e confirmar objetos ignorados.

Esses componentes devem receber o snapshot neutro da Bancada, enquanto adaptadores
Anycubic, Bambu e outros traduzem protocolos para o mesmo modelo. Nao devemos
copiar o JavaScript proprietario nem assumir que os comandos de uma marca servem
para outra.

## Proximas investigacoes REA

- Reiniciar a sessao Codex e confirmar `binary_session`/`open_binary` no servidor.
- Abrir `Workbench.dll` uma unica vez e pesquisar strings menores por cada modulo.
- Inspecionar referencias e funcoes associadas a `WorkbenchV3_vue.js`,
  `icon-skip`, `axisControlContext` e `deviceManagement`.
- Analisar os assets web embutidos e comparar dimensoes, nomes e grupos de icones.
- Usar observacao de runtime somente se a evidencia estatica nao responder uma
  pergunta especifica; nao conectar nem comandar a Kobra 4 Combo ocupada.
- Implementar a proxima revisao no 3T3D como componentes visuais, nao como mais
  linhas ad hoc no painel atual.

## Limites da evidencia

Strings e caminhos de modulo sao observacoes estaticas do DLL e nao provam, por
si so, o fluxo de execucao ou o protocolo de rede. A imagem enviada pelo usuario
continua sendo a referencia visual de layout; REA acrescenta a arquitetura
embutida e os nomes de componentes que devem orientar a reconstruçao.
