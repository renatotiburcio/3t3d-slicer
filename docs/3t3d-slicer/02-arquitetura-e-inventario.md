# Arquitetura e inventário

## Base técnica

O 3T3D Slicer mantém a arquitetura do OrcaSlicer: aplicação C++17, GUI wxWidgets, CMake, núcleo de fatiamento em `src/libslic3r`, GUI em `src/slic3r/GUI`, dependências compiladas separadamente e perfis declarativos em `resources/profiles`.

| Camada | Local principal | Papel | Regra de customização |
|---|---|---|---|
| Inicialização | `src/OrcaSlicer.cpp`, `src/slic3r/GUI/GUI_App.cpp` | ciclo de vida, configuração, idioma e janela | mudanças pequenas, testadas em primeiro boot e upgrade |
| Núcleo | `src/libslic3r/` | modelos, geometria, fatiamento, G-code, perfis | manter compatibilidade `.3mf`, determinismo e testes de domínio |
| GUI | `src/slic3r/GUI/` | Plater, presets, preview, calibração e diálogos | strings completas e traduzíveis; não duplicar lógica de domínio |
| Configuração | `src/libslic3r/PrintConfig.*`, `PresetBundle.*` | opções e seleção de presets | versionar migrações e validar defaults |
| Recursos | `resources/`, `assets/` | perfis, ícones, modelos de mesa, traduções compiladas | validar JSON, nomes e empacotamento |
| Build | `CMakeLists.txt`, `cmake/`, `deps/`, scripts | dependências, executáveis e instaladores | manter Windows, Linux e macOS |
| Testes | `tests/` | Catch2 e ferramentas auxiliares | registrar no CTest e incluir casos de regressão |

## Inventário local observado

- Anycubic: 56 JSON de máquina, 115 de processo e 183 de filamento na árvore atual, além de modelos/texturas de mesa.
- Imagens: alterações binárias e vetoriais em vários nomes ainda herdados de OrcaSlicer.
- Perfil de marca: `version.inc` e `GUI_App.cpp` têm customizações de produto/idioma.
- Build: Visual Studio 18 2026, configuração Release no cache; o diretório também contém saídas Debug/MinSizeRel, sinal de que o build precisa ser normalizado antes da automação.
- Documentação upstream: `docs/HLSD/` já contém descrições de subsistemas do OrcaSlicer; a documentação de projeto foi isolada nesta pasta para não misturar design upstream com planejamento do 3T3D.

## Fluxo de dados crítico

1. O usuário abre um projeto ou importa uma malha.
2. `PresetBundle` resolve impressora, processo e filamentos compatíveis.
3. `Print`/`PrintObject` preparam geometria e opções.
4. Núcleos de perímetro, preenchimento, suporte, brim, costura e G-code produzem o resultado.
5. Preview, exportação ou agente de impressora consomem o G-code.
6. Projeto, presets e metadados precisam continuar legíveis por versões futuras.

Qualquer recurso importado de outro fork deve ser encaixado nesse fluxo, e não copiado apenas como tela ou bloco isolado.

## Pontos de atenção arquiteturais

- Recursos Bambu dependem frequentemente de fluxo de rede, autenticação, AMS, câmeras, firmware e convenções de G-code; eles não devem ser ativados genericamente sem uma camada de capacidade por fabricante.
- Recursos Anycubic devem ser separados entre perfil de máquina, transporte/integração de dispositivo, calibração e UI proprietária.
- O núcleo deve permanecer independente de marca sempre que possível; a marca deve entrar por interfaces, perfis e adaptadores.
- Compatibilidade de presets exige nomes estáveis, variantes de nozzle, `compatible_printers` válidos e migração explícita.
