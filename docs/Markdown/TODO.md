# Roadmap de Conclusao do Projeto

## Fase 1: Modelo e Arquivo Unico (CONCLUIDO)
- [x] UserRecord como classe base C++
- [x] StudentRecord com campos extendidos (email, cpf, curso, ano)
- [x] TeacherRecord (departamento, especializacao, contratacao)
- [x] SubjectRecord (nome, codigo, creditos, professor)
- [x] Arquivo unico records.dat com header + chunks
- [x] FileManager generico (recordSize como parametro)

## Fase 2: Indexacao (CONCLUIDO)
- [x] Indice hash com chaves STU:/TCH:/SUB:/NM:
- [x] Indice atualizado a cada insercao/remocao
- [x] IDs incrementais por tipo de entidade
- [x] Arvore B+ como estrutura de matriculas
- [x] Chaves compostas (ENR:STU:NNNN:SUB:MMMM)
- [x] Consulta por prefixo (matriculas de um aluno)

## Fase 3: Views Lua (CONCLUIDO)
- [x] StudentCreate com novos campos
- [x] StudentList e StudentDetail atualizados
- [x] TeacherCreate e TeacherList
- [x] SubjectCreate e SubjectList
- [x] EnrollmentList (matricular, notas, consultas)

## Fase 4: Crescimento Dinamico
- [ ] Reallocacao automatica de chunks (capacity * 2)
- [ ] Reescrita do arquivo com novos offsets
- [ ] Testes de estresse com >100 registros

## Fase 5: Melhorias
- [ ] Busca por nome com indice hash (NM:)
- [ ] Desmatricular (remover da B+ tree)
- [ ] Atualizar dados de entidades existentes
- [ ] Interface de edicao de registros

## Fase 6: Testes e Documentacao
- [ ] Testes unitarios para StudentRecord
- [ ] Testes unitarios para TeacherRecord
- [ ] Testes unitarios para SubjectRecord
- [ ] Testes de insercao/busca na arvore B+
- [ ] Testes de reallocacao de chunks
- [ ] Diagramas UML atualizados
- [ ] Documentacao do formato binario

*Documento criado para o projeto TP AEDS III - Roadmap*
