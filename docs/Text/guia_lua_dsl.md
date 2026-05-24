# Guia Lua DSL - Sistema de Matricula AEDS III

## API do DataManager

Todas as operacoes sao acessadas via `getDataManager()`:

```lua
local dm = getDataManager()
```

### Aluno

| Metodo | Parametros | Retorno |
|--------|-----------|---------|
| `createStudent` | name, email, cpf, birthDate, courseName, enrollmentYear | bool |
| `readStudent` | id | StudentRecord ou nil |
| `deleteStudent` | id | bool |
| `listAllStudents` | - | StudentRecord[] |

### Professor

| Metodo | Parametros | Retorno |
|--------|-----------|---------|
| `createTeacher` | name, email, cpf, department, specialization, hireDate | bool |
| `readTeacher` | id | TeacherRecord ou nil |
| `deleteTeacher` | id | bool |
| `listAllTeachers` | - | TeacherRecord[] |

### Disciplina

| Metodo | Parametros | Retorno |
|--------|-----------|---------|
| `createSubject` | name, code, credits, teacherId | bool |
| `readSubject` | id | SubjectRecord ou nil |
| `deleteSubject` | id | bool |
| `listAllSubjects` | - | SubjectRecord[] |

### Matricula (Arvore B+)

| Metodo | Parametros | Retorno |
|--------|-----------|---------|
| `enrollStudent` | studentId, subjectId, teacherId, semester | bool |
| `getEnrollment` | studentId, subjectId | BTreeLeafValue ou nil |
| `updateGrade` | studentId, subjectId, grade | bool |
| `unenroll` | studentId, subjectId | bool |
| `getEnrollmentsByStudent` | studentId | BTreeLeafValue[] |
| `getEnrollmentsBySubject` | subjectId | BTreeLeafValue[] |

### Info

| Metodo | Parametros | Retorno |
|--------|-----------|---------|
| `getActiveCount` | type ('S', 'T', 'B') | int |
| `getLastError` | - | string |
| `getNextStudentId` | - | int |
| `getNextTeacherId` | - | int |
| `getNextSubjectId` | - | int |

### Exemplos

```lua
-- Cadastrar aluno
local dm = getDataManager()
dm:createStudent("Joao Silva", "joao@email.com", "12345678901", 15051990, "Ciencia Comp", 2026)

-- Listar professores
local teachers = dm:listAllTeachers()
for i, t in ipairs(teachers) do
    imgui.Text(t.name .. " - " .. t.department)
end

-- Matricular
dm:enrollStudent(1, 1, 1, "2026-1")

-- Consultar matriculas de um aluno
local enrollments = dm:getEnrollmentsByStudent(1)
for i, e in ipairs(enrollments) do
    imgui.Text("Disciplina " .. e.subjectId .. " | Nota: " .. e.grade)
end
```

*Documento criado para o projeto TP AEDS III - Guia Lua DSL*
