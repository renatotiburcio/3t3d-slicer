# Roadmap e prioridades

## P0 — tornar o estado reproduzível

1. Congelar uma fotografia de referência: commit upstream, toolchain, CMake, dependências e configuração.
2. Separar branding, perfis, filamentos, build e experimentos em mudanças independentes.
3. Corrigir ou formalizar a política do conjunto `addnorth`; não silenciar o validador.
4. Fazer o CTest registrar e executar as suítes esperadas; documentar o motivo de qualquer exclusão.
5. Revalidar JSON e seleção de todos os perfis Anycubic, começando pela Kobra 4.

## P1 — primeira versão interna utilizável

1. Primeiro boot limpo: nome 3T3D, pt-BR, ícone, wizard e persistência de preferências.
2. Importar, fatiar, salvar e reabrir projetos `.3mf` sem perda.
3. Gerar G-code para cada família de impressora suportada e comparar com uma referência conhecida.
4. Completar pacote Windows e registrar instalação, atualização, desinstalação e execução.
5. Criar smoke tests de GUI e testes de perfil sem depender de uma impressora conectada.

## P2 — consolidação multi-fabricante

1. Comparar sistematicamente recursos BambuStudio e AnycubicSlicerNext por commit/tag.
2. Portar apenas recursos com valor para o produto, usando adaptadores de capacidade.
3. Integrar agentes de impressora, AMS e monitoramento somente após revisão de segurança e privacidade.
4. Criar migrações de perfil e projeto quando nomes/estruturas divergirem.
5. Manter um relatório de divergência com o OrcaSlicer upstream para facilitar rebases.

## P3 — release público

- testes Windows/macOS/Linux;
- builds reprodutíveis e artefatos assinados;
- revisão de licenças e avisos dos três upstreams;
- documentação de suporte e recuperação;
- telemetria opt-in, se existir, com política explícita;
- matriz de impressoras e materiais suportados com versão dos perfis.

## Critério de pronto por recurso

Um recurso só avança para “consolidado” quando possui implementação isolada, documentação de design quando necessária, teste positivo, teste de opção desabilitada, compatibilidade de projeto/perfil, verificação de tradução e evidência de build nas plataformas afetadas.
