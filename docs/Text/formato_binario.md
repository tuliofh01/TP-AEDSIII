# Formato Binário - Sistema de Matrícula AEDS III

## Visão Geral

O sistema utiliza persistência binária com registros de tamanho fixo de **67 bytes** cada. Não há necessidade de bancos de dados externos - tudo é armazenado diretamente em arquivos binários.

---

## Estrutura do Registro (67 bytes)

| Offset | Tamanho | Tipo | Campo | Descrição |
|--------|---------|------|-------|------------|
| 0 | 1 | char | status | 'A' = Ativo, '*' = Deletado |
| 1 | 4 | int32_t | id | ID dinâmico (recalculado) |
| 5 | 4 | int32_t | userId | ID do usuário no sistema |
| 9 | 50 | char[50] | name | Nome do estudante |
| 59 | 4 | uint32_t | birthDate | Data de nascimento (YYYYMMDD) |
| 63 | 4 | - | padding | Bytes de alinhamento |

**Total: 67 bytes por registro**

---

## Representação Visual

```
Byte:  0        1         5         9                    59    63    67
       ├────────┼─────────┼─────────┼─────────────────────┼─────┼────┤
Campo: │ status │    id   │  userId │        name         │birth│pad │
       │  [1]   │   [4]   │   [4]   │        [50]         │ [4] │[4] │
       └────────┴─────────┴─────────┴─────────────────────┴─────┴────┘
```

---

## Tipos C++

O sistema usa tipos de `<cstdint>` para portabilidade:

```cpp
char status;           // 1 byte (char)
int32_t id;           // 4 bytes (signed 32-bit)
int32_t userId;       // 4 bytes (signed 32-bit)
char name[50];        // 50 bytes (array char)
uint32_t birthDate;   // 4 bytes (unsigned 32-bit)
```

---

## Soft Delete

O sistema não remove registros fisicamente. Em vez disso:
- Marca o registro com status = '*' (Deletado)
- Não sobrescreve o arquivo
- Próximas operações ignoram registros deletados

Vantagens:
- Não precisa reescrever todo o arquivo
- Recuperação possível (alterar '*' de volta para 'A')
- Histórico preservado

---

## Operações de Serialização

### Write (C++ → Binário)

```cpp
auto StudentRecord::toBytes() const -> std::vector<std::byte> {
    std::vector<std::byte> bytes(67);
    // Campo a campo para evitar padding issues
    bytes[0] = static_cast<std::byte>(status);
    std::memcpy(bytes.data() + 1, &id, 4);
    std::memcpy(bytes.data() + 5, &userId, 4);
    std::memcpy(bytes.data() + 9, name, 50);
    std::memcpy(bytes.data() + 59, &birthDate, 4);
    return bytes;
}
```

### Read (Binário → C++)

```cpp
auto StudentRecord::fromBytes(const std::vector<std::byte>& data) -> StudentRecord {
    StudentRecord rec;
    if (data.size() >= 67) {
        rec.status = static_cast<char>(data[0]);
        std::memcpy(&rec.id, data.data() + 1, 4);
        std::memcpy(&rec.userId, data.data() + 5, 4);
        std::memcpy(rec.name, data.data() + 9, 50);
        std::memcpy(&rec.birthDate, data.data() + 59, 4);
    }
    return rec;
}
```

---

## Arquivos de Dados

### students.dat
- Contém os registros binários (67 bytes cada)
- Cresce adicionando registros no final
- Arquivo de tamanho variável

### students.idx
- Índice hash para busca por nome
-binário com estrutura:
  - Magic number (4 bytes): 0x494E4445 ('IND E')
  - Profundidade (4 bytes)
  - Mapa de hash (nome → offset)

---

## Critérios de Portabilidade

1. **Tipos fixos**: `int32_t`, `uint32_t` em vez de `int`, `long`
2. **Sem pointers**: Dados diretamente nos registros
3. **Sem std::string**: Arrays char[] de tamanho fixo
4. **Endianness**: Assume little-endian (comum em x86/x64)

---

## Validações

- Nome não pode ser vazio
- ID mínimo é 1
- Data de nascimento no formato YYYYMMDD

---

*Documento criado para o projeto TP AEDS III - Formato Binário*