# Validação de warnings e perfis addnorth

## Resultado

Em 2026-11-17, a correção foi implementada e validada no Windows Release.

- `OrcaSlicer_profile_validator.exe -p resources/profiles -v Anycubic`: código 0.
- Saída: `Total loaded vendors: 73` e `Validation completed successfully`.
- `OrcaSlicer_profile_validator.exe -p resources/profiles`: código 0; todos os 73 vendors foram carregados.
- Os JSONs dos perfis `addnorth` não precisaram ser alterados: as referências para impressoras Bambu existentes eram válidas.

## Correções aplicadas

- Removida captura de exceção sem uso em `src/libslic3r/Config.cpp`.
- Substituídas contagens `std::count(..., true)` em vetores `unsigned char` por `std::count_if` em `PresetBundle.cpp` e `OrcaSlicer.cpp`.
- Removidos três `[[unlikely]]` incompatíveis com C++17 em `TextureToColor.cpp`.
- Migrado o hash de upload de `PrinterFileSystem.cpp` das APIs MD5 depreciadas do OpenSSL 3 para `boost::uuids::detail::md5`, já usado no download.
- Isolado o falso positivo C4723 emitido pelo cabeçalho wxWidgets com `/wd4723` apenas no alvo GUI.
- Removido `/MT` forçado de `deps_src/md4c/CMakeLists.txt`; o runtime agora acompanha o `/MD` do aplicativo, eliminando LNK4098.
- Removido `/LTCG` global sem geração de código do `CMakeLists.txt` principal.
- O modo de validação agora carrega todos os vendors mesmo com `-v`; o argumento continua restringindo o sweep/report, enquanto referências cruzadas permanecem resolvíveis.

## Build

O alvo `OrcaSlicer_profile_validator` foi recompilado com CMake 4.4.4/MSBuild 18.11.1. Após forçar o rebuild de `md4c` e `md4c-html`, o link não reportou mais LNK4098 nem o aviso `/LTCG sem geração de código`.

O build completo anterior teve uma execução interrompida após gerar as bibliotecas e a DLL; o alvo do validador foi então recompilado isoladamente e validado com sucesso. A próxima reconstrução completa deve ser usada como confirmação final de todos os executáveis após as alterações de runtime/link.
