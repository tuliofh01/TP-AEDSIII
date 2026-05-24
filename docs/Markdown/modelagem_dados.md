# Modelagem de Dados (DER) - Sistema de Matricula Academica

## Relacoes de Dados

| Origem | Destino | Cardinalidade | Descricao |
|--------|---------|---------------|-----------|
| **User** | **Student** | 1:1 | Especializacao de usuario como aluno |
| **User** | **Teacher** | 1:1 | Especializacao de usuario como professor |
| **Teacher** | **Subject** | 1:N | Um professor ministra varias disciplinas |
| **Student** | **Subject** | N:N | Matricula via arvore B+ (enrollment) |

## Estrutura de Dados

### UserRecord (classe base C++, nao persistida diretamente)
- status, type, userId, name, email, cpf

### StudentRecord (persistido no chunk 'S')
- Herda UserRecord
- birthDate, courseName, enrollmentYear

### TeacherRecord (persistido no chunk 'T')
- Herda UserRecord
- department, specialization, hireDate

### SubjectRecord (persistido no chunk 'B')
- status, subjectId, name, code, credits, teacherId

### Matricula (armazenada na arvore B+)
- Chave: `ENR:STU:<id>:SUB:<id>`
- Valor: studentId, subjectId, teacherId, grade, semester

## Diferencas do Modelo Anterior

1. **Arquivo unico** — todas as entidades em `records.dat` com chunks internos
2. **Sem EnrollmentRecord** — matricula e a propria entrada na arvore B+
3. **Indice hash** — busca rapida por ID ou nome, atualizado a cada insercao
4. **UserRecord como base** — Student e Teacher compartilham campos comuns
5. **Crescimento dinamico** — chunks duplicam de tamanho quando cheios

*Documento criado para o projeto TP AEDS III - Modelagem de Dados*
