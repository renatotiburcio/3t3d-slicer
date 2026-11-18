# Comparativo dos repositórios de referência

## Linhagem

O OrcaSlicer é a base imediata deste workspace. O BambuStudio e o AnycubicSlicerNext compartilham a linhagem PrusaSlicer/Slic3r; o AnycubicSlicerNext declara explicitamente que foi baseado no OrcaSlicer e que o OrcaSlicer veio originalmente do BambuStudio. Isso torna a comparação útil, mas também aumenta o risco de copiar alterações já divergentes ou dependentes de patches proprietários.

| Projeto | Papel no 3T3D | O que deve ser extraído | O que não deve ser presumido |
|---|---|---|---|
| OrcaSlicer | base de código, UX e pipeline | arquitetura atual, features gerais, testes e perfis | que tudo do upstream está integrado/validado no fork |
| BambuStudio | referência para integração Bambu e fluxos de produção | capacidades de printer agent, AMS, envio, monitoramento, perfis BBL e UX específica | que protocolos, servidores ou licenças podem ser portados sem auditoria |
| AnycubicSlicerNext | referência Anycubic sobre a mesma família de código | perfis Anycubic, nomenclatura, integração de máquinas e ajustes de marca | que uma feature proprietária é segura ou compatível com todas as impressoras |
| 3T3D Slicer | produto consolidado | marca, seleção de recursos, compatibilidade multi-fabricante e governança | que “ter o arquivo” equivale a ter a feature pronta |

## Estratégia de comparação

As páginas oficiais foram consultadas em 2026-11-17. O GitHub identifica o OrcaSlicer como gerador de G-code para várias famílias de impressoras e mostra sua árvore com `src`, `resources`, `tests` e `docs`. O BambuStudio se apresenta como software de fatiamento rico em recursos e sua árvore possui `bbl/i18n`, `resources`, `src`, testes e ferramentas próprias. O AnycubicSlicerNext declara ser um slicer open source baseado no OrcaSlicer e expõe uma árvore estrutural semelhante.

A comparação de código deve ser feita por commit/tag, com `git diff --no-index` ou clones em diretórios separados, e deve registrar:

- origem exata (commit/tag e caminho);
- dependências e macros envolvidas;
- diferença de formato de projeto/preset;
- impacto no G-code e no firmware;
- licença e avisos preservados;
- testes portados e teste negativo quando a opção estiver desabilitada.

## Recursos candidatos por origem

### BambuStudio

Prioridade de investigação: perfis BBL e compatibilidade de variantes; agentes de impressora; envio, monitoramento e filas; AMS/mapeamento de filamentos; tratamentos de G-code e fluxos de multi-material; ferramentas e testes em `bbs_test_tools`.

Esses itens são candidatos, não compromissos. Recursos de rede devem permanecer atrás de capacidades explícitas, autenticação segura, timeout, logs sem credenciais e uma política de privacidade.

### AnycubicSlicerNext

Prioridade de investigação: perfis de máquina/processo/filamento Anycubic, modelos de mesa, defaults e nomenclatura; integração de dispositivo quando aplicável; diferenças de calibração e UX que não existem no OrcaSlicer.

O primeiro resultado já presente no workspace é a ampliação de perfis Anycubic, incluindo Kobra 4. A próxima etapa é provar que cada preset é selecionável, compatível e capaz de gerar G-code correto para a máquina alvo.

### OrcaSlicer

O upstream deve continuar sendo a referência para evolução do núcleo, testes, documentação HLSD, compatibilidade de formatos e atualização de dependências. O 3T3D deve manter um relatório de divergência para evitar que a customização de marca impeça a incorporação de correções de segurança ou bugs.

## Fontes

- [OrcaSlicer](https://github.com/OrcaSlicer/OrcaSlicer)
- [BambuStudio](https://github.com/bambulab/BambuStudio)
- [AnycubicSlicerNext](https://github.com/ANYCUBIC-3D/AnycubicSlicerNext)
