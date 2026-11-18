# 3T3D Slicer — identidade, assistente inicial e provedores de nuvem

## Diagnóstico da demora no assistente

O assistente é implementado por `GuideFrame` em `src/slic3r/GUI/WebGuideDialog.cpp` e renderiza as páginas de `resources/web/guide/`. A tela de carregamento aparecia porque `OnNavigationComplete()` construía o JSON completo de perfis no mesmo thread da interface, percorrendo fornecedores, modelos, impressoras, filamentos e processos antes de exibir o WebView.

Esse trabalho é especialmente caro na primeira execução, quando o cache `cache/wizard_profile_data.json` ainda não existe. O efeito visual era uma janela branca com o spinner ativo; depois que a construção terminava, a página de boas-vindas aparecia.

O caminho atual inicia o carregamento em `m_load_task`, registra início e duração no log e entrega o resultado ao thread da interface com `CallAfter`. O cache continua sendo reutilizado quando suas marcas de fornecedor são válidas. Assim, o WebView pode exibir o estado de carregamento sem bloquear a janela. O próximo teste obrigatório é medir uma instalação limpa e uma segunda execução, procurando as mensagens `GuideFrame: profile loading started on worker thread` e `GuideFrame: profile loading completed in ... ms`.

## Identidade 3T3D

O assistente usa o logo rasterizado de `resources/web/image/logo.png`, os textos de boas-vindas do 3T3D Slicer e a paleta verde do logo (`#1aa870`, com variações de hover e foco). A mesma identidade foi aplicada às folhas de estilo do guia, ao estado de carregamento e aos estilos globais do WebView.

Ainda existem identificadores internos e nomes de compatibilidade contendo `OrcaSlicer`, porque eles participam de caminhos, formatos, testes, nomes de targets e compatibilidade de projetos. Eles não devem ser substituídos em massa sem separar:

- texto visível ao usuário, que deve dizer `3T3D Slicer`;
- nome de fornecedor ou produto externo, que deve permanecer quando for tecnicamente correto;
- identificador interno de compatibilidade, que deve permanecer até existir migração formal.

## Escolha de conexão

O assistente não apresenta mais opções de Cloud. A homepage também não exibe login, contas, Bambu Cloud ou a entrada de Cloud no sidebar. A interface permanece local/offline e a fábrica mantém os identificadores internos apenas para compatibilidade com agentes e projetos existentes; eles não são expostos ao usuário.

O provedor Anycubic possui um identificador reservado (`anycubic`), mas não há um `ICloudServiceAgent` Anycubic nesta árvore. A integração Anycubic será tratada prioritariamente por LAN e só será exposta quando houver agente real, autenticação, armazenamento de credenciais, endpoints, renovação de sessão, erros, logout e testes contra o serviço real.

## Detecção de impressoras Anycubic em rede

A ausência de detecção é esperada no estado atual: `OrcaPrinterAgent::start_discovery()` é um stub e não existe um agente Anycubic registrado em `NetworkAgentFactory`. Ter o perfil da impressora não cria descoberta nem comunicação de rede.

A Kobra 4 possui um modo LAN próprio no Anycubic Slicer Next; a documentação da Anycubic descreve esse modo como necessário para comunicação local. O protocolo observado por implementações independentes usa um documento HTTP local em `:18910/info` e depois MQTT/TLS para controle. Isso é uma integração diferente de Bambu Cloud e exige um agente Anycubic LAN separado, com descoberta, identificação, autenticação/handshake e operações de impressão. A implementação não deve ser substituída por um simples “ping” ou por uma varredura bloqueante da rede.

## Pendências

1. Validar o tempo do assistente em instalação limpa, cache válido e cache inválido.
2. Abrir o aplicativo e confirmar visualmente a tela de seleção, o logo 3T3D e a ausência de `OrcaSlicer` nos textos do onboarding.
3. Validar Bambu Cloud com o plugin carregado e com uma impressora Bambu selecionada.
4. Implementar o adaptador Anycubic Cloud somente após definir a API oficial/permitida e o modelo de credenciais.
5. Implementar o agente Anycubic LAN/Cloud e conectá-lo ao callback de descoberta; validar com uma Kobra 4 em LAN Mode.
6. Auditar os demais textos visíveis, especialmente homepage, preferências, mensagens de atualização e telas de login; não alterar nomes de APIs, formatos ou caminhos sem migração.
7. Atualizar os catálogos de tradução após estabilizar os novos textos do assistente.

## Perfil Anycubic e Kobra 4

Os arquivos da Kobra 4 Combo estão no fornecedor `Anycubic`, incluindo modelo, variante de nozzle, processo e geometria da mesa. O carregador do assistente agora compara a versão da cópia instalada no diretório de dados com a versão bundled em `resources/profiles` e não deixa uma cópia antiga esconder uma versão nova. Quando não existe seleção anterior, o guia pré-seleciona o identificador compatível `Anycubic Kobra 4`, apresentado na interface como `Anycubic Kobra 4 Combo`.

Além da versão, o carregador verifica a completude dos arquivos filhos. Isso evita o caso em que `%APPDATA%/OrcaSlicer/system/Anycubic.json` possui um manifesto atualizado, mas não possui os arquivos `machine/`, `process/` ou a geometria da Kobra 4. A área útil do perfil prioritário é 260×260×260 mm, com nozzle padrão de 0,4 mm; variantes de nozzle opcionais só devem ser habilitadas após validar seus limites e perfis de processo.

## Critério de conclusão

O assistente é considerado corrigido quando a janela permanece responsiva durante a carga, o log registra uma duração finita, a segunda execução usa o cache, a identidade visual é 3T3D e a escolha Bambu/3T3D encaminha para o agente correspondente. Anycubic só será marcado como concluído quando houver agente real e teste de autenticação/conexão.

## Ajustes de inicialização atuais

O assistente não passa mais pela página de região/login: a primeira etapa útil é a seleção de impressoras. O cache derivado da lista recebeu uma nova versão de formato para invalidar dados antigos que poderiam ocultar a Kobra 4 Combo. A página de carregamento mantém apenas um fallback limitado para WebView, e a tela de boas-vindas não usa rolagem vertical.

Na homepage foram removidos o link Wiki, a consulta automática de recomendações de Design e os recursos visuais associados àquela área. Projetos recentes, novo projeto e abertura de projeto permanecem intactos.

## Catálogo de filamentos

O catálogo `OrcaFilamentLibrary` foi atualizado para a versão `02.04.00.28`. A lista inclui os novos arquivos de marcas e materiais no bundle, incluindo 3D Lab, Cliever, Creality, F3D, Voolt e os novos perfis eSUN. O carregador do assistente agora verifica os `sub_path` de `machine`, `process` e `filament` antes de aceitar uma árvore instalada; uma cópia incompleta em `%APPDATA%` não pode mais ocultar os filamentos bundled.

Também foi corrigido o tratamento de falha parcial: um fornecedor instalado de forma incompleta não aborta mais a montagem de todos os fornecedores. Os vendors carregados com sucesso, especialmente Anycubic, continuam disponíveis enquanto o fornecedor problemático é registrado no log para diagnóstico.
# Correção recente — impressoras Anycubic

Os logs de 17/11/2026 mostraram que a lista de filamentos era montada, mas um arquivo vazio/corrompido no diretório de sistema do usuário fazia o `PresetBundle` omitir todo o fornecedor Anycubic. O cache derivado ainda era aceito porque continha impressoras de outras marcas, perpetuando a ausência da Kobra 4.

O carregador agora rejeita cache sem modelos Anycubic, detecta árvores de fornecedor com arquivos ausentes ou vazios e, se o bundle ainda omitir Anycubic por erro legado, carrega o manifesto bundled de `resources/profiles/Anycubic.json` diretamente. A validação após a correção confirmou 22 modelos Anycubic, uma variante de máquina Kobra 4 e 7.660 filamentos no cache; a Kobra 4 aparece internamente como `Anycubic Kobra 4 0.4 nozzle` e é apresentada como Combo na interface.

A primeira reconstrução após invalidar o cache ainda percorreu o caminho de recuperação legado e levou aproximadamente 47,9 s. Nas próximas aberturas o cache válido deve reduzir esse tempo para a ordem de centenas de milissegundos; caso contrário, a duração deve ser investigada separadamente.

# Correção recente — capa e preset padrão da Kobra 4

O perfil agora inclui `resources/profiles/Anycubic/Anycubic Kobra 4_cover.png`, usando a imagem oficial da Kobra 4 Combo e o nome de arquivo que o sidebar procura a partir de `printer_model`. O carregamento de um manifesto Anycubic antigo sem os arquivos de máquina da Kobra 4 passou a ser considerado incompleto: o app instala o bundle bundled antes de carregar os presets. Quando o preset ativo ainda é `Default Printer`, o app seleciona `Anycubic Kobra 4 0.4 nozzle`, grava a seleção e atualiza o sidebar; escolhas explícitas de outras impressoras não são substituídas.

# Auditoria de conectividade Anycubic e bancada — 2026-11-17

## Evidência de protocolo encontrada

Foi localizado o projeto open-source `ianloic/anycubic-orca-plugin`, mantido em `.tools/anycubic-orca-plugin` apenas como material de auditoria. Ele documenta uma implementação comunitária do protocolo LAN Anycubic:

- descoberta SSDP em UDP/HTTP na porta `18910`;
- consulta `GET /info`;
- autenticação local por `POST /ctrl`, com desafio e credenciais MQTT derivadas;
- MQTT TLS na porta `9883` para telemetria e comandos;
- upload direto por `/gcode_upload`;
- câmera HTTP FLV na porta `18088`;
- telemetria de temperaturas, progresso, estado, ventiladores e ACE Pro.

O projeto informa testes reais principalmente com Kobra S1 e Kobra 3. A Kobra 4 aparece no catálogo do plugin, mas ainda não há validação independente neste workspace. Por isso, o código não foi copiado para o produto nem habilitado automaticamente.

### Plano de integração

1. Adaptar o cliente Python para o ABI de plugin local já existente no 3T3D Slicer, mantendo todo I/O de rede em workers.
2. Registrar o agente como `anycubic` e associá-lo ao perfil Kobra 4 sem alterar projetos antigos.
3. Validar primeiro descoberta, `/info` e leitura de estado; só depois habilitar upload, impressão, ACE, câmera e controles.
4. Executar teste real com a Kobra 4 Combo em modo LAN e registrar logs de sucesso/erro.


## Estado atual

### Distinção dos modelos Anycubic

Os perfis `Anycubic Kobra 4` e `Anycubic Kobra 4 Combo` são modelos independentes. Ambos possuem entrada própria no seletor, máquina e preset de 0,4 mm, além de capas próprias. O Combo é o padrão inicial do workspace porque corresponde ao equipamento principal em uso; o Kobra 4 continua selecionável separadamente. O processo padrão é compatível com os dois presets.

Foi identificado um erro na primeira consolidação: o arquivo de máquina do Combo existia, mas não estava listado em `machine_list` do manifesto `Anycubic.json`. Por isso o arquivo era instalado, porém o seletor não o carregava. O manifesto foi corrigido e sua versão incrementada para forçar a atualização do perfil instalado.

### Navegação 3T3D

A aba Design/CAD foi retirada da navegação principal. O código CAD legado permanece compilável para não afetar componentes compartilhados, mas não cria página nem item de menu no executável 3T3D. Prepare, Preview, Device e Project continuam sendo os espaços ativos.

- O perfil da Anycubic Kobra 4 0.4 nozzle é instalado e selecionado como padrão quando a árvore de perfis Anycubic está ausente ou incompleta.
- A miniatura oficial `resources/profiles/Anycubic/Anycubic Kobra 4_cover.png` está disponível para o seletor e para a barra lateral.
- A carga de perfis em cache já foi validada em aproximadamente 1,6 s no carregamento completo; a primeira reconstrução continua sendo o caminho lento e deve permanecer fora da interface principal.
- A comparação com o `AnycubicSlicerNext` não encontrou um agente LAN Anycubic/Kobra reutilizável. O núcleo de rede continua baseado em agentes Bambu/Orca, Moonraker e outros adaptadores.

## Correção de comportamento enganoso

`OrcaPrinterAgent` era um stub que retornava sucesso para descoberta, conexão, envio de comandos e impressão sem executar transporte algum. Ele agora retorna `ORCA_NETWORK_ERR_CMD_NOT_SUPPORTED` nas operações não implementadas e `start_discovery()` não anuncia mais uma descoberta falsa. A desconexão continua idempotente.

Isso evita que a bancada mostre uma impressora como conectada ou aceite uma ação de impressão quando nenhum protocolo foi negociado.

## Próxima implementação recomendada

1. Capturar, com a Kobra 4 Combo ligada, o tráfego de descoberta e de sessão LAN produzido pelo Anycubic Slicer Next ou pelo aplicativo oficial, respeitando autenticação e privacidade.
2. Identificar o contrato mínimo: descoberta, estado, temperaturas, câmera, envio de arquivo/G-code, pausa, retomada, cancelamento e controle de eixos.
3. Criar `AnycubicPrinterAgent` separado, com testes de parsing e testes de integração opcionais; não reutilizar comandos JSON Bambu por semelhança nominal.
4. Usar a bancada como camada de apresentação comum: cartões de tarefa, controles, câmera, materiais e informações do dispositivo devem ser habilitados conforme as capacidades do agente, e não por marca.

## Bancada de trabalho

`MonitorPage` ainda é apenas um painel vazio; a funcionalidade real está distribuída entre `MonitorPanel`, `DeviceManager`, `MachineObject` e componentes de dispositivo. A consolidação da bancada deve começar pela matriz de capacidades do agente, para que recursos inexistentes na Kobra 4 não apareçam como controles ativos.

## Seletor de impressoras

O popup do seletor agora exibe a pesquisa em todas as plataformas, não somente no Windows. A busca é case-insensitive e considera o nome da impressora e o endereço IP, permitindo localizar rapidamente a Kobra 4 quando houver mais de um dispositivo.

O cache do thumbnail não armazena mais o placeholder. Isso permite que a barra lateral tente novamente quando os perfis terminarem de carregar, além de procurar diretamente a capa Anycubic durante esse estado transitório.

O popup também deixou de descartar silenciosamente o pedido de descoberta: ao abrir/fechar, ele encaminha `start_discovery()` ao agente ativo e registra se o agente aceitou. Com o agente legado Orca o resultado continuará negativo, de forma honesta; com um futuro agente Anycubic real a mesma integração poderá ser ativada sem alterar a UI.

`WebViewPanel::SendCloudProvidersInfo()` agora envia uma lista vazia deliberadamente. Assim, configurações antigas não conseguem reativar a conta ou a barra lateral de Cloud na homepage.

