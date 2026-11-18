# 3T3D Slicer — bancada unificada e catálogo de modelos

## Solicitação registrada

A tela “Bancada de trabalho” do Anycubic Slicer Next deve ser analisada como referência para uma área única de operação de impressoras. A primeira implementação deve ser orientada à Anycubic Kobra 4 Combo, sem esconder as diferenças de protocolo, hardware ou capacidade.

## Elementos observados na referência Anycubic

- seleção de impressora em nuvem e LAN;
- cartão de tarefa com miniatura, progresso, camadas, tempo restante, pausa, cancelamento e finalização;
- controles de temperatura, câmera, ventilação, modo de impressão e detecção;
- informações do dispositivo, firmware, modelo e número de série;
- câmera ao vivo;
- gestão de materiais/ACE e secagem;
- movimentação dos eixos com passos configuráveis e aviso de segurança.

## Consolidação proposta

| Área comum | 3T3D/Orca | Bambu | Anycubic |
|---|---|---|---|
| Tarefa e progresso | agente/protocolo disponível conforme impressora | Bambu LAN/Cloud | depende do adaptador Anycubic LAN/Cloud |
| Temperaturas e estado | somente capacidades reportadas | suporte amplo via agente Bambu | validar endpoints e permissões |
| Câmera | capability `CameraStreamMode` | câmera LAN/Cloud | URL/stream específico por modelo/firmware |
| Materiais | filamentos do perfil | AMS e bandejas | ACE/ACE Pro e secagem, quando disponíveis |
| Eixos | capability explícita e intertravamento | comandos do agente Bambu | comandos Anycubic precisam ser validados |
| Firmware | agente pode reportar atualização | fluxo Bambu | fluxo Anycubic separado |

O painel deve ser orientado por capacidades, não por fabricante. Um cartão só aparece quando o agente informa que a capacidade existe; quando não existe, o controle deve ficar oculto ou mostrar “não disponível”, nunca enviar comandos de outro protocolo.

## Arquitetura recomendada

1. `DeviceWorkbenchModel`: estado normalizado da impressora, tarefa, câmera, materiais e capacidades.
2. `IPrinterAgent`: operações por capability, com retorno explícito de “não suportado”.
3. `WorkbenchView`: painel comum e cartões condicionais.
4. Adaptadores `3T3D/Orca`, `Bambu` e `Anycubic` isolando protocolos e autenticação.
5. Atualizações assíncronas com cancelamento, debounce e cache curto para a interface nunca bloquear durante descoberta ou consulta.

## Pesquisa de modelos na página inicial

A página inicial deverá oferecer busca federada por fontes como MakerWorld, MakerOnline e Printables. Cada resultado deve conter fonte, título, miniatura, autor, popularidade/data quando fornecida, tags e URL canônica. O botão de detalhes deve abrir a página original no navegador padrão.

Não devemos fazer scraping indiscriminado no thread da interface nem copiar modelos/arquivos para dentro do aplicativo. A implementação deve usar adaptadores por fonte, limites de requisição, cache, timeout, cancelamento e fallback; quando uma fonte não fornecer API estável, mostrar somente busca externa ou abrir a página no navegador. Isso evita quebrar o app por mudanças de HTML e respeita os termos de cada serviço.

## Pesquisa da seleção de impressoras

A janela de seleção deve ter campo de pesquisa por nome, fornecedor e modelo, com filtragem incremental no lado da interface. O filtro deve operar sobre o índice já carregado, sem reler JSONs a cada tecla. A listagem inicial deve carregar em segundo plano e apresentar fornecedores/modelos progressivamente quando possível.

## Pendências de implementação

- criar o modelo de capacidades da bancada;
- definir o contrato de estado normalizado;
- analisar os componentes equivalentes do Bambu Studio e Anycubic Slicer Next em código-fonte;
- implementar o filtro de pesquisa da seleção de impressoras;
- construir o catálogo federado de modelos com abertura externa;
- implementar o agente Anycubic LAN para a Kobra 4 Combo antes de habilitar controles reais;
- validar descoberta, estado, tarefa, câmera, temperatura, materiais ACE 2 Pro e comandos de movimento em uma impressora real;
- validar cada cartão em uma impressora real e registrar capacidades por modelo/firmware.
