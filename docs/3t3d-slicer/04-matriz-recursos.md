# Matriz de recursos

Legenda: **confirmado** = evidência no código/recursos; **relatado** = aparece na sessão, mas precisa de confirmação; **pendente** = ainda não implementado ou bloqueado; **não avaliado** = requer comparação de código e teste.

| Domínio | Origem candidata | Situação no 3T3D | Próxima evidência necessária |
|---|---|---|---|
| Pipeline de slicing | OrcaSlicer | confirmado como base | baseline de G-code e regressões |
| GUI/Plater/Preview | OrcaSlicer | confirmado como base | smoke test Windows e snapshots/roteiro |
| Branding, ícones e nome | customização 3T3D | SVGs, PNGs e ICOs sincronizados; pacote completo ainda pendente | revisar metadados, instalador e nomes herdados |
| pt-BR inicial | customização 3T3D | confirmado no código; catálogo/runtime não revalidados nesta auditoria | abrir em perfil limpo e verificar todas as telas |
| Anycubic Kobra 4 Combo | AnycubicSlicerNext/customização | perfil presente, validado pelo profile validator; validação em máquina real pendente | selecionar, fatiar, exportar e comparar G-code em máquina real |
| Demais perfis Anycubic | AnycubicSlicerNext | presentes em grande parte da árvore | validar JSON, nozzle, mesa, processo e filamentos por família |
| Filamentos brasileiros | customização 3T3D | arquivos presentes para vários fornecedores | revisar temperaturas, custos, compatibilidade e origem dos dados |
| Default Kobra 4 Combo | sessão/customização | seleção automática implementada no assistente; teste de primeiro boot em ambiente limpo pendente | confirmar seleção, filamento e processo padrão |
| Perfis BBL | BambuStudio | existentes parcialmente na base; inconsistência `addnorth` confirmada | alinhar bundle ou remover referências órfãs, sem mascarar erro |
| AMS/multi-material | BambuStudio | não avaliado como integração 3T3D | comparar fluxo completo, G-code, UI e compatibilidade |
| Printer Agent/rede | Orca/Bambu/Anycubic | não avaliado | matriz de capacidades, autenticação, TLS, timeout e testes offline |
| Câmera/monitoramento | Bambu/Anycubic | não avaliado | política de privacidade, endpoints e testes com credenciais fictícias |
| Calibrações | Orca/Bambu/Anycubic | base Orca confirmada; complementos não catalogados | inventariar telas, G-code gerado e persistência |
| Formatos `.3mf`/projeto | OrcaSlicer | herdado, não revalidado após customizações | round-trip em versão anterior, atual e futura |
| Plugins/extensões | Orca/Bambu | não avaliado | permissões, sandbox, API e compatibilidade de versão |
| Instaladores e atualização | customização 3T3D | pendente | build limpa, assinatura, upgrade/uninstall e antivírus |
| Testes automatizados | OrcaSlicer | infraestrutura existe, mas CTest local registra 0 | configurar/registrar testes e publicar comandos reproduzíveis |

## Recursos que ainda não devem ser marcados como “consolidados”

Qualquer item de BambuStudio ou AnycubicSlicerNext que dependa de firmware, rede, contas, cloud, AMS, câmera, suporte proprietário ou G-code específico permanece não avaliado até haver comparação de implementação e teste controlado.
