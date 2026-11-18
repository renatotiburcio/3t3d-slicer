# Estado atual do 3T3D Slicer

## Resumo executivo

O workspace é, neste momento, um checkout do OrcaSlicer em `main` (`59acdedd06`, tag `nightly-builds`) com customizações locais ainda não commitadas. A base do slicer está presente e o build Release Windows foi recompilado com sucesso, mas o produto ainda não constitui uma distribuição 3T3D validada: não existe uma suíte de testes registrada no CTest, a validação completa dos perfis falha e a integração de recursos específicos de BambuStudio/AnycubicSlicerNext ainda não foi consolidada como módulos rastreáveis.

## Confirmado no workspace

- Branding local: `SLIC3R_APP_NAME` foi alterado para `3T3D Slicer`, enquanto `SLIC3R_APP_KEY` permanece `OrcaSlicer` para preservar o nome do catálogo de tradução.
- Idioma inicial: `GUI_App.cpp` fornece `pt_BR` quando a configuração ainda não contém idioma.
- Identidade visual: os SVG novos de `assets/` foram sincronizados com os nomes consumidos pelo app em `resources/images/`: ícone para `OrcaSlicer.svg`/splash, `OrcaSlicer_gradient*`, `OrcaSlicer_gray.svg` e `OrcaSlicer_gradient_circle.svg`, horizontal claro para About/branding claro e variante escura derivada com texto branco. Os PNGs de 32, 44, 50, 64, 128, 150, 154, 192 e 256 px, as variantes transparente/grayscale, os ativos MSIX e os ICOs também foram regenerados a partir do novo ícone. O SVG vertical permanece como ativo-fonte disponível, mas não é referenciado por uma tela atual do app.
- Correção de rasterização: a primeira geração dos PNGs capturou barras de rolagem do navegador; os arquivos foram regenerados capturando somente o SVG inline, com `overflow:hidden`, transparência e dimensões exatas. O `OrcaSlicer.icns` também foi reconstruído com entradas PNG de 16, 32, 48, 128, 256, 512 e 1024 px.
- Anycubic: `resources/profiles/Anycubic.json` foi alterado e a árvore contém presets de máquina, processo, filamento e modelos de mesa para a Kobra 4 Combo, além dos perfis Anycubic já existentes. O perfil prioritário usa 260×260×260 mm e 0,4 mm como nozzle padrão.
- Filamentos: há adições locais para `3D Lab`, `Cliever`, `Creality`, `F3D`, `Voolt` e novos/alterados presets eSUN; a biblioteca contém 35 diretórios de fornecedores na fotografia atual.
- Build: `build/CMakeCache.txt` aponta para Visual Studio 18 2026 com configuração `Release`; o rebuild foi concluído com CMake 4.4.4/MSBuild 18.11.1 e gerou `orca-slicer.exe`, `OrcaSlicer.dll` e `OrcaSlicer_profile_validator.exe` em `build/src/Release`.
- A sessão anterior registra preparação do ambiente, configuração, build e tentativas de execução, mas o transcript contém falhas de comandos e não substitui a validação atual.

## Confirmado como pendência ou bloqueio

- `OrcaSlicer_profile_validator.exe -p resources\\profiles -v Anycubic` passa com código 0. A pendência de validação independente dos perfis `addnorth` deve continuar sendo tratada separadamente, sem bloquear a validação Anycubic.
- `ctest --test-dir build -N` informa `Total Tests: 0`; portanto o build existente não prova a saúde dos testes do produto.
- Os binários são chamados `orca-slicer.exe`/`OrcaSlicer.dll`; a existência do branding no código não prova que instalador, atalhos, metadados e nome de pacote tenham sido migrados.
- A árvore tem muitas modificações não relacionadas entre si e arquivos não rastreados. Antes de novas alterações, cada tema deve ser isolado em mudanças revisáveis.
- Não há, nesta árvore, um catálogo auditável de funcionalidades importadas do BambuStudio ou AnycubicSlicerNext. A matriz em `04-matriz-recursos.md` é o ponto de partida, não uma declaração de paridade.

## Em andamento nesta fotografia

1. Separação da base upstream, customizações 3T3D e recursos candidatos de outros forks.
2. Auditoria de compatibilidade dos perfis e correção do conjunto `addnorth` em mudança independente.
3. Definição da matriz de recursos, critérios de aceitação e cobertura de testes antes de novas integrações.
4. Preparação de uma identidade de produto completa: nome, ícones, tradução, executável, instalador e metadados.

## Estado de entrega

**Não pronto para release.** O caminho mínimo para uma primeira versão interna é: separar mudanças, corrigir/decidir a política para `addnorth`, registrar testes reais, validar carregamento dos perfis e executar o app com um perfil Kobra 4 em ambiente limpo. O caminho para release público inclui ainda compatibilidade de projeto `.3mf`, migração de perfis, licenças, segurança, empacotamento e testes multiplataforma.

## Evidência consultada

- `git status --short`, `git diff --stat`, `version.inc`, `src/slic3r/GUI/GUI_App.cpp`.
- `resources/profiles/Anycubic/`, `resources/profiles/OrcaFilamentLibrary/` e `build/CMakeCache.txt`.
- `session/sessao.md`, usado como histórico de trabalho e não como prova única.
