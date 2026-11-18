# Bancada Anycubic, simulador e inicializacao

## Estado em 2026-11-18

Esta etapa foi feita sem descoberta LAN, sem MQTT, sem login cloud e sem comando para a impressora real.

### Contrato de estado

`src/slic3r/Utils/AnycubicWorkbenchState.hpp` define um snapshot neutro para a futura Bancada de trabalho:

- identificacao do modelo, nome, numero de serie e host;
- estados desconectada, conectando, ociosa, preparando, imprimindo, pausada, concluida e erro;
- camada atual, total de camadas e percentual;
- temperaturas atual e alvo do bico e da mesa;
- camera, ACE e quantidade de slots;
- mensagem de erro.

O contrato foi mantido independente do protocolo Anycubic. O adaptador LAN real devera apenas traduzir seus eventos para esse snapshot, sem acoplar a UI ao formato de MQTT, HTTP ou SSDP.

### Simulador local

`AnycubicKobra4Simulator` cria uma Kobra 4 Combo simulada com camera e quatro slots ACE, permite alterar estado, progresso e temperaturas e avancar o progresso com `tick()`. O host usa o esquema `simulator://` e nenhum metodo abre socket.

Isso permite desenvolver a Bancada, validar estados e produzir screenshots enquanto a impressora real estiver ocupada.

### Bancada visual e ignorar objeto

A primeira aba `Bancada` foi adicionada ao `MainFrame` com os blocos de tarefa, controle,
camera, materiais e informacoes do dispositivo. A organizacao segue a referencia enviada
do Anycubic Slicer Next, mas usa controles nativos do aplicativo e permanece independente
de cloud.

Na revisao visual, a aba foi reorganizada em tres colunas: impressoras, tarefa/camera e
controle/dispositivo. A tarefa agora exibe a placa simulada, os objetos e a relacao
explicita `ID -> nome`, evitando que o usuario precise adivinhar a qual peca pertence um
identificador. As strings da Bancada usam conversao UTF-8 explicita para evitar mojibake
em Windows com configuracoes regionais diferentes.

O simulador oferece tres objetos de teste e a acao `Ignorar objeto...`. A acao permite
selecionar varias pecas, exige confirmacao e registra cada objeto uma unica vez. Ela nao
publica rede nem altera a impressora real.

O caminho real devera reutilizar a infraestrutura ja existente: o fatiamento gera
`EXCLUDE_OBJECT_DEFINE`, `EXCLUDE_OBJECT_START` e `EXCLUDE_OBJECT_END`, enquanto o agente
de dispositivo ja possui o comando `skip_objects`. Antes de habilitar esse comando para a
Kobra 4 Combo, sera necessario confirmar no protocolo Anycubic o indice dos objetos, o
estado permitido, a resposta de erro e a persistencia apos pausa/retomada. A implementacao
real deve exigir impressora conectada, tarefa imprimindo e confirmacao do usuario.

### Auditoria do Anycubic Slicer Next

A arvore aberta do Anycubic Slicer Next foi pesquisada por `workbench`, `skip_objects`,
`exclude_object`, camera, materiais e controle de eixos. Os componentes nativos e o
protocolo de impressao estao presentes, mas a tela de Bancada mostrada nas referencias
nao aparece como uma implementacao nativa completa nessa arvore; parte dela depende da
camada de dispositivo/UI proprietaria. A consolidacao no 3T3D deve usar os padroes
abertos de wxWidgets e os adaptadores de dispositivo, sem copiar uma dependencia
proprietaria ou presumir que o protocolo seja identico para todas as marcas.

A selecao de objetos foi evoluida para um dialogo proprio com pre-visualizacao da placa,
IDs desenhados nas pecas, lista correspondente `ID -> nome`, selecao/desselecao e
confirmacao. A viewport 3D real do modo Preparar continua como proxima integracao: ela
devera ser reutilizada apenas como visualizador/selecionador, sem permitir transformacao
ou alteracao do projeto durante uma impressao.

### DNA visual 3T3D

A Bancada usa a paleta extraida dos SVG oficiais em `assets`: ciano `#1DBDDF`, verde
`#1AA870`, magenta `#D11F77`, preto `#211E1F` e branco. Ciano identifica telemetria
e acoes primarias; verde identifica estado operacional; magenta identifica acoes
potencialmente destrutivas; preto estrutura cabecalhos e areas de camera.

A unidade da Bancada mantem o codigo-fonte ASCII e converte textos com escapes Unicode
por `wxString::FromUTF8`, evitando que configuracoes ANSI do Windows produzam mojibake.
Os cards, hierarquia, espacamento, estados e acoes devem seguir essa paleta e semantica,
em vez de usar controles wxWidgets com a aparencia padrao como identidade final.

### Paridade visual da etapa atual

O dashboard agora usa componentes visuais dedicados nos pontos que a referencia do
Anycubic torna mais importantes: a tarefa possui uma miniatura compacta da placa e
numeracao dos objetos; a Gestao de materiais exibe quatro rolos ACE desenhados com
cores e posicoes; e Informacoes do dispositivo carrega o ativo oficial
`profiles/Anycubic/Anycubic Kobra 4 Combo_cover.png`. O painel Controle mantem as acoes
de simulacao isoladas e a Bancada inclui o bloco Mover eixo com incrementos e direcoes
desabilitados enquanto nao houver conexao real, evitando comandos acidentais.

Esses elementos sao dados da UI/simulador, nao uma promessa de suporte de protocolo.
Quando o adaptador Anycubic for integrado, a mesma composicao devera receber telemetria
real, estados de firmware, camera, tarefa e materiais sem alterar a semantica visual.

O acabamento dos cards segue tokens compartilhados: 12 px de distancia entre blocos,
10 px de padding interno, raio de 12 px, borda cinza de baixo contraste e fundo externo
cinza-claro. Isso evita que cada painel crie seu proprio ritmo e deixa a grade estavel
quando a janela muda de tamanho. Os cards continuam com superficie branca e acoes
coloridas apenas onde existe significado operacional.

### Startup

O assistente de configuracao agora e aberto antes da inicializacao opcional do agente de rede. A sincronizacao de plugins cloud foi removida do caminho de startup. Depois que o fluxo modal do assistente termina, a rede/plugin e inicializada de forma atrasada para preservar LAN futura sem bloquear a tela de boas-vindas.

Tambem foram atualizados textos visiveis em erros fatais, WebView2, atualizacao, idioma e metadados do Windows para 3T3D Slicer. Identificadores internos de compatibilidade, como o esquema de associacao `orcaslicer`, permanecem preservados.

### Validacao

- CMake 4.4.4 regenerou o projeto `Visual Studio 18 2026`.
- `OrcaSlicer_app_gui.vcxproj` Release compilou com sucesso.
- A aba inicial `Bancada` foi adicionada ao aplicativo com tarefa, miniatura, progresso, camadas, temperaturas, camera simulada, ACE visual, imagem da Kobra 4 Combo, dispositivo e controles seguros.
- `slic3rutils_tests.exe "[Anycubic][Workbench]"` passou: 6 casos e 28 assertions.
- O warning `LNK4098` nao reapareceu.
- Nenhuma conexao com a Kobra 4 Combo foi executada.

### Proximas etapas

- Refinar a composicao com dados reais do projeto: thumbnail renderizada da tarefa, nomes e cores efetivas dos filamentos e metadados do preset selecionado.
- Reutilizar a viewport 3D do modo Preparar no seletor de ignorar objetos, mantendo uma camada de interacao exclusiva para selecao/desselecao.
- Validar visualmente em diferentes escalas DPI e tamanhos de janela, incluindo o primeiro carregamento.
- Quando a impressora estiver livre, validar a traducao do protocolo Anycubic e substituir o simulador somente no caminho real.
