# Academic Enrollment System (AEDS III)

Este projeto é um sistema de gerenciamento acadêmico focado em persistência em memória secundária, utilizando arquivos binários com indexação simples e manipulação direta de registros de tamanho fixo.

## 🚀 Destaques de C++ Moderno (C++20)

O projeto foi refatorado para seguir as melhores práticas do C++ moderno:
- **C++20 Standard:** Configurado via CMake para garantir acesso às funcionalidades mais recentes.
- **Gerenciamento de Memória:** Uso de `std::unique_ptr` para transferência de propriedade segura e `std::make_unique` para evitar vazamentos.
- **Tipagem Robusta:** Utilização de `<cstdint>` (`std::int32_t`, `std::uint32_t`) para garantir portabilidade binária entre Linux e Windows.
- **Dedução de Tipos:** Uso intensivo de `auto` para simplificar declarações complexas e melhorar a legibilidade.
- **Filesystem:** Manipulação de arquivos e diretórios utilizando as bibliotecas padrão do sistema de arquivos.

## 🛠️ Instruções de Build

O projeto utiliza **CMake** e é compatível com Linux e Windows.

### Pré-requisitos
- Compilador com suporte a C++20 (GCC 10+, Clang 10+, ou MSVC 2019+)
- CMake 3.10 ou superior

### Compilação (Linux/macOS)
```bash
mkdir -p builds && cd builds
cmake ..
make
```

### Compilação (Windows - MinGW)
```powershell
mkdir builds; cd builds
cmake -G "MinGW Makefiles" ..
mingw32-make
```

### Execução
Os executáveis finais são gerados na pasta `builds/`:
- **Sistema:** `./builds/enrollment_system`
- **Testes Unitários:** `./builds/run_tests`

## 🧠 Lógica e Funcionamento

1. **Persistência Binária:** Os dados são armazenados em `data/students.dat`. O arquivo possui um cabeçalho de 4 bytes contendo o último ID gerado, seguido por registros sequenciais do tipo `Student`.
2. **StudentDAO:** Implementa o padrão Data Access Object. As operações são realizadas via `fstream`, movendo o ponteiro do arquivo (`seekg`/`seekp`) para as posições exatas dos registros.
3. **Exclusão Lógica:** Ao "excluir" um estudante, o registro não é removido fisicamente do arquivo para manter a integridade das posições. Em vez disso, a flag `removed` é marcada como `true`.
4. **FileManager:** Classe utilitária estática responsável por inicializar arquivos e gerenciar o controle de IDs sequenciais.

## 📁 Estrutura de Documentação
- `docs/UML Diagrams/`: Diagramas de fluxo, casos de uso e relacionamentos (Mermaid).
- `docs/modern_cpp_tutorial.md`: Guia de estilo e boas práticas aplicadas ao projeto.
- `docs/modelagem_dados.md`: Detalhes técnicos sobre as entidades e o DER.
- `tests/`: Suíte de testes unitários para validação de integridade.

## 👥 Contribuidores

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/victorynogueira">
        <img src="https://github.com/victorynogueira.png" width="100px;" alt="victorynogueira"/><br />
        <sub><b>victorynogueira</b></sub>
      </a>
    </td>
  </tr>
</table>
