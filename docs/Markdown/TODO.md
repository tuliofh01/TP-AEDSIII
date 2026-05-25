# Roadmap de Conclusao do Projeto

## Fase 1: Modelo e Arquivo Unico (CONCLUIDO)
- [x] UserRecord como classe base C++
- [x] StudentRecord com campos extendidos (email, cpf, curso, ano)
- [x] TeacherRecord (departamento, especializacao, contratacao)
- [x] SubjectRecord (nome, codigo, creditos, professor)
- [x] Arquivo unico records.dat com header + chunks
- [x] FileManager generico (recordSize como parametro)

## Fase 2: Indexacao (CONCLUIDO)
- [x] Indice hash persistente com chaves STU:/TCH:/SUB:/NM:
- [x] Indice atualizado a cada insercao/remocao
- [x] IDs incrementais por tipo de entidade (nextId)
- [x] Arvore B+ como estrutura de matriculas
- [x] Chaves compostas (ENR:STU:NNNN:SUB:MMMM)
- [x] Consulta por prefixo (matriculas de um aluno/disciplina)
- [x] Update grade na B+ tree (erase + insert)

## Fase 3: Views Lua (CONCLUIDO)
- [x] StudentCreate com novos campos (email, cpf, curso, ano)
- [x] StudentList e StudentDetail atualizados
- [x] TeacherCreate e TeacherList
- [x] SubjectCreate e SubjectList
- [x] EnrollmentList (matricular, notas, consultas por aluno/disciplina)
- [x] Sidebar com navegacao e contagem de ativos

## Fase 4: Crescimento Dinamico (CONCLUIDO)
- [x] Reallocacao automatica de chunks (capacity * 2)
- [x] Reescrita do arquivo com novos offsets
- [x] B+ tree chunkOffset_ atualizado apos realloc

## Fase 5: Melhorias (CONCLUIDO)
- [x] Busca por nome com indice hash (NM:<nome>)
- [x] Desmatricular (remove da B+ tree)
- [x] Atualizar nota (erase + insert)

## Fase 6: Em Andamento
- [x] Testes unitarios (10 testes: CRUD + enrollment)
- [x] Diagramas UML atualizados (classes, arquitetura, binario, sequencia)
- [x] Documentacao do formato binario (formato_binario.md)
- [ ] Testes de reallocacao de chunks (>100 registros)
- [ ] Interface de edicao de registros existentes

*Documento criado para o projeto TP AEDS III - Roadmap*
