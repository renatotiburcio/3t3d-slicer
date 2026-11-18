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

### Startup

O assistente de configuracao agora e aberto antes da inicializacao opcional do agente de rede. A sincronizacao de plugins cloud foi removida do caminho de startup. Depois que o fluxo modal do assistente termina, a rede/plugin e inicializada de forma atrasada para preservar LAN futura sem bloquear a tela de boas-vindas.

Tambem foram atualizados textos visiveis em erros fatais, WebView2, atualizacao, idioma e metadados do Windows para 3T3D Slicer. Identificadores internos de compatibilidade, como o esquema de associacao `orcaslicer`, permanecem preservados.

### Validacao

- CMake 4.4.4 regenerou o projeto `Visual Studio 18 2026`.
- `OrcaSlicer_app_gui.vcxproj` Release compilou com sucesso.
- A aba inicial `Bancada` foi adicionada ao aplicativo com tarefa, progresso, camadas, temperaturas, camera simulada, ACE, dispositivo e controles seguros.
- `slic3rutils_tests.exe "[Anycubic][Workbench]"` passou: 3 casos e 18 assertions.
- O warning `LNK4098` nao reapareceu.
- Nenhuma conexao com a Kobra 4 Combo foi executada.

### Proximas etapas

- Criar a pagina visual da Bancada usando o snapshot e o simulador.
- Conectar os cards de tarefa, controle, camera, materiais e informacoes do dispositivo.
- Validar visualmente a aba `Bancada` no executavel Release.
- Quando a impressora estiver livre, validar a traducao do protocolo Anycubic e substituir o simulador somente no caminho real.
