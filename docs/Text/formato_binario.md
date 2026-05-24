# Formato Binario - Arquivo Unico com Chunks

## Visao Geral

O sistema utiliza um unico arquivo binario (`records.dat`) com layout de cabecalho + chunks de tamanho fixo. Cada chunk armazena um tipo de entidade (aluno, professor, disciplina) ou paginas da arvore B+ para matriculas.

Um segundo arquivo (`records.idx`) contem o indice hash para busca rapida por ID e nome.

---

## Estrutura do Arquivo Unico

```
records.dat:

[0-255]: FILE HEADER (256 bytes)
  ├── magic (4B): 0x52454331 ('REC1')
  ├── version (4B): 1
  ├── headerSize (4B): 256
  ├── chunkCount (4B): 4
  ├── nextStudentId (4B)
  ├── nextTeacherId (4B)
  ├── nextSubjectId (4B)
  ├── padding (4B)
  └── reserved (224B):
      ├── ChunkInfo[0] (32B) — Aluno
      ├── ChunkInfo[1] (32B) — Professor
      ├── ChunkInfo[2] (32B) — Disciplina
      └── ChunkInfo[3] (32B) — Arvore B+

[256..]: CHUNK ALUNO (100 registros × 139B)
[..]:    CHUNK PROFESSOR (100 registros × 165B)
[..]:    CHUNK DISCIPLINA (100 registros × 84B)
[..]:    CHUNK ARVORE B+ (4 paginas × 4096B)
```

---

## ChunkInfo (32 bytes cada)

| Offset | Tamanho | Tipo | Campo |
|--------|---------|------|-------|
| 0 | 1 | char | type ('S', 'T', 'B', 'I') |
| 1 | 3 | - | padding |
| 4 | 8 | uint64_t | offset (byte offset no arquivo) |
| 12 | 4 | uint32_t | recordSize |
| 16 | 4 | uint32_t | capacity |
| 20 | 4 | uint32_t | used |
| 24 | 8 | - | padding |

---

## Registro de Aluno (StudentRecord, 139 bytes)

| Offset | Tamanho | Tipo | Campo |
|--------|---------|------|-------|
| 0 | 1 | char | status ('A' ativo, '*' deletado) |
| 1 | 1 | char | type ('S') |
| 2 | 4 | int32_t | userId |
| 6 | 50 | char[50] | name |
| 56 | 30 | char[30] | email |
| 86 | 15 | char[15] | cpf |
| 101 | 4 | uint32_t | birthDate (DDMMAAAA) |
| 105 | 30 | char[30] | courseName |
| 135 | 4 | int32_t | enrollmentYear |

---

## Registro de Professor (TeacherRecord, 165 bytes)

| Offset | Tamanho | Tipo | Campo |
|--------|---------|------|-------|
| 0 | 1 | char | status |
| 1 | 1 | char | type ('T') |
| 2 | 4 | int32_t | userId |
| 6 | 50 | char[50] | name |
| 56 | 30 | char[30] | email |
| 86 | 15 | char[15] | cpf |
| 101 | 30 | char[30] | department |
| 131 | 30 | char[30] | specialization |
| 161 | 4 | uint32_t | hireDate |

---

## Registro de Disciplina (SubjectRecord, 84 bytes)

| Offset | Tamanho | Tipo | Campo |
|--------|---------|------|-------|
| 0 | 1 | char | status |
| 1 | 1 | char | type ('B') |
| 2 | 4 | int32_t | subjectId |
| 6 | 50 | char[50] | name |
| 56 | 20 | char[20] | code |
| 76 | 4 | int32_t | credits |
| 80 | 4 | int32_t | teacherId |

---

## Arvore B+ (Enrollment = Matriculas)

A arvore B+ armazena matriculas **dentro de suas folhas** — nao ha EnrollmentRecord separado.

### Cabecalho do Chunk (24 bytes, antes das paginas)

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 0 | 8 | rootPageId |
| 8 | 8 | entryCount |
| 16 | 8 | pageCount |

### Pagina (4096 bytes)

**Cabecalho de pagina (31 bytes):**

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 0 | 1 | type ('I' internal, 'L' leaf) |
| 1 | 2 | numKeys |
| 3 | 8 | parentPageId |
| 11 | 8 | nextLeafPageId (folhas) |
| 19 | 8 | prevLeafPageId (folhas) |
| 27 | 4 | pageId |

**Entrada em folha (48 bytes):**
- Chave (20B): `"ENR:STU:0042:SUB:0015"`
- Valor (28B):

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 20 | 4 | studentId |
| 24 | 4 | subjectId |
| 28 | 4 | teacherId |
| 32 | 4 | grade (float) |
| 36 | 12 | semester |

**Entrada em no interno (28 bytes):**
- childPtr (8B) + separatorKey (20B)

### Capacidade
- Folha: ~84 entradas por pagina
- Interno: ~144 chaves, 145 filhos (fanout)
- Altura tipica: 2-3 niveis para 10.000 matriculas

---

## Indice Hash (records.idx)

Arquivo separado com estrutura:

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 0 | 4 | magic (0x494E4445 = 'INDE') |
| 4 | 4 | depth |
| 8 | 4 | numEntries |
| 12 | ... | entries (len + key + chunkIndex + recordIndex) |

**Chaves do indice:**

| Chave | Exemplo | Busca |
|-------|---------|-------|
| `STU:<userId>` | `STU:42` | Aluno por ID |
| `TCH:<userId>` | `TCH:7` | Professor por ID |
| `SUB:<id>` | `SUB:15` | Disciplina por ID |
| `NM:<nome>` | `NM:joao silva` | Busca por nome |

---

## Reallocacao (crescimento dinamico)

Quando `used >= capacity` em qualquer chunk:

1. Capacidade do chunk e duplicada
2. Arquivo inteiro e reescrito com novos offsets
3. Arvore B+ tem seu chunkOffset_ atualizado
4. Indice hash permanece valido (recordIndex nao muda)

---

## Soft Delete

Registros nao sao removidos fisicamente:
- Status byte alterado para `'*'`
- Operacoes de leitura ignoram registros com status `'*'`
- Espaco so e reutilizado na proxima reallocacao

---

*Documento criado para o projeto TP AEDS III - Formato Binario*
