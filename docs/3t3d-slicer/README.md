# 3T3D Slicer — documentação de engenharia

Esta pasta é a fonte de acompanhamento do 3T3D Slicer. O produto é construído sobre o checkout do OrcaSlicer e deve consolidar, quando houver justificativa técnica e compatibilidade, recursos encontrados no BambuStudio e no AnycubicSlicerNext.

## Documentos

- [Estado atual e evidências](01-estado-atual.md) — o que existe hoje, o que foi confirmado e o que permanece incerto.
- [Arquitetura e inventário](02-arquitetura-e-inventario.md) — mapa do código, dados, build, testes e pontos de extensão.
- [Comparativo dos repositórios](03-comparativo-repositorios.md) — relação entre OrcaSlicer, BambuStudio, AnycubicSlicerNext e o 3T3D Slicer.
- [Matriz de recursos](04-matriz-recursos.md) — recursos por domínio, origem candidata, estado no 3T3D e critério de aceitação.
- [Roadmap e backlog](05-roadmap-e-prioridades.md) — ordem de execução, dependências e riscos.
- [Validação e governança](06-validacao-e-governanca.md) — como manter a análise atualizada e impedir regressões.

## Regra de atualização

Cada mudança relevante deve atualizar `01-estado-atual.md` e o documento de domínio correspondente. Um item só pode ser marcado como **concluído** quando houver evidência no código e uma validação reproduzível. Relatos da sessão `session/sessao.md` sem confirmação no workspace devem permanecer como **relatados** ou **pendentes**.

Data desta fotografia: 2026-11-17.
