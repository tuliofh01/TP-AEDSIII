# Arquitetura GUI - Sistema de Matricula AEDS III

## Visao Geral

Interface grafica com HelloImGui + Lua como DSL (Domain Specific Language).

---

## Estrutura de Camadas

```
┌─────────────────────────────────────────┐
│           HelloImGui Runner            │
│  (Janela, loop, eventos)               │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│         Lua Router (router.lua)         │
│  (Navegacao entre views)                │
└────────────────┬────────────────────────┘
                 │
        ┌───────┼────────┬────────┬──────────┐
        ▼       ▼        ▼        ▼         ▼
    ┌──────┐ ┌───────┐ ┌────┐ ┌────────┐ ┌──────┐
    │Main  │ │Student│ │Teach│ │Subject │ │Enroll│
    │Menu  │ │Views  │ │Views│ │Views   │ │Views │
    └──────┘ └───────┘ └────┘ └────────┘ └──────┘
```

---

## Views Disponiveis

| View | Arquivo | Funcao |
|------|---------|--------|
| Main Menu | `MainMenu.lua` | Menu inicial com resumo e acoes rapidas |
| Cadastrar Aluno | `StudentCreate.lua` | Formulario de cadastro (nome, email, cpf, data, curso, ano) |
| Listar Alunos | `StudentList.lua` | Lista com exclusao |
| Consultar Aluno | `StudentDetail.lua` | Busca por ID |
| Cadastrar Professor | `TeacherCreate.lua` | Formulario de cadastro (nome, email, cpf, depto, especializacao) |
| Listar Professores | `TeacherList.lua` | Lista com exclusao |
| Cadastrar Disciplina | `SubjectCreate.lua` | Formulario (nome, codigo, creditos, professor) |
| Listar Disciplinas | `SubjectList.lua` | Lista com exclusao |
| Gerenciar Matriculas | `EnrollmentList.lua` | Matricular, notas, consultar por aluno/disciplina |

---

## Tema 4 Cores

As mesmas 4 cores do tema original: Preto, Branco, Verde, Vermelho.

---

## DataManager Binding

Todas as operacoes CRUD sao expostas para Lua via `luaaa`:
- Aluno: `createStudent`, `readStudent`, `deleteStudent`, `listAllStudents`
- Professor: `createTeacher`, `readTeacher`, `deleteTeacher`, `listAllTeachers`
- Disciplina: `createSubject`, `readSubject`, `deleteSubject`, `listAllSubjects`
- Matricula: `enrollStudent`, `updateGrade`, `unenroll`, `getEnrollmentsByStudent`, `getEnrollmentsBySubject`

*Documento criado para o projeto TP AEDS III - Arquitetura GUI*
