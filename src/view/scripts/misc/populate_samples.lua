-- populate_samples.lua
-- Cria dados de amostra complexos: professores, alunos, disciplinas, matriculas e notas

local M = {}

function M.populate(dm)
	-- ============================================================
	-- PROFESSORES
	-- ============================================================

	-- Prof. 1: DCC - Banco de Dados (CPF: 00000000000, senha: 1234)
	local t1 = dm:createTeacher("Dr. Carlos Alberto Mendes", "carlos@escola.com",
		"00000000000", 1234, "DCC", "Banco de Dados", 20050301)
	if t1 then print("Professor 1 criado (CPF: 00000000000)") end

	-- Prof. 2: DCC - Estruturas de Dados (CPF: 11111111112, senha: 1234)
	local t2 = dm:createTeacher("Prof. Ana Lucia Oliveira", "ana@escola.com",
		"11111111112", 1234, "DCC", "Estruturas de Dados", 20100301)
	if t2 then print("Professor 2 criado (CPF: 11111111112)") end

	-- Prof. 3: DEMAT - Matematica (CPF: 22222222223, senha: 1234)
	local t3 = dm:createTeacher("Dr. Roberto Santos Lima", "roberto@escola.com",
		"22222222223", 1234, "DEMAT", "Matematica Aplicada", 20080301)
	if t3 then print("Professor 3 criado (CPF: 22222222223)") end

	-- ============================================================
	-- ALUNOS
	-- ============================================================

	-- Aluno 1 (CPF: 11111111111, senha: 1234) - Ciencia da Computacao
	local s1 = dm:createStudent("Joao Pedro Silva", "joao@aluno.com",
		"11111111111", 1234, 20020515, "Ciencia da Computacao", 2026)
	if s1 then print("Aluno 1 criado (CPF: 11111111111, ID 1)") end

	-- Aluno 2 (CPF: 22222222222, senha: 1234) - Engenharia de Software
	local s2 = dm:createStudent("Maria Clara Oliveira", "maria@aluno.com",
		"22222222222", 1234, 20030820, "Engenharia de Software", 2025)
	if s2 then print("Aluno 2 criado (CPF: 22222222222, ID 2)") end

	-- Aluno 3 (CPF: 33333333333, senha: 1234) - Sistemas de Informacao
	local s3 = dm:createStudent("Pedro Henrique Costa", "pedro@aluno.com",
		"33333333333", 1234, 20011110, "Sistemas de Informacao", 2024)
	if s3 then print("Aluno 3 criado (CPF: 33333333333, ID 3)") end

	-- Aluno 4 (CPF: 44444444444, senha: 1234) - Ciencia da Computacao
	local s4 = dm:createStudent("Ana Beatriz Souza", "ana.b@aluno.com",
		"44444444444", 1234, 20040705, "Ciencia da Computacao", 2026)
	if s4 then print("Aluno 4 criado (CPF: 44444444444, ID 4)") end

	-- Aluno 5 (CPF: 55555555555, senha: 1234) - Engenharia de Software
	local s5 = dm:createStudent("Lucas Santos Pereira", "lucas@aluno.com",
		"55555555555", 1234, 20021201, "Engenharia de Software", 2024)
	if s5 then print("Aluno 5 criado (CPF: 55555555555, ID 5)") end

	-- ============================================================
	-- DISCIPLINAS
	-- ============================================================

	-- Disciplina 1: Banco de Dados (prof 1)
	local bdOk = dm:createSubject("Banco de Dados", "BD001", 60, 1)
	if bdOk then print("Disciplina 'Banco de Dados' (ID 1, prof 1)") end

	-- Disciplina 2: Logica de Programacao (prof 1)
	local lpOk = dm:createSubject("Logica de Programacao", "LP001", 60, 1)
	if lpOk then print("Disciplina 'Logica de Programacao' (ID 2, prof 1)") end

	-- Disciplina 3: Estruturas de Dados (prof 2)
	local edOk = dm:createSubject("Estruturas de Dados", "ED001", 60, 2)
	if edOk then print("Disciplina 'Estruturas de Dados' (ID 3, prof 2)") end

	-- Disciplina 4: Algoritmos Avancados (prof 2)
	local aaOk = dm:createSubject("Algoritmos Avancados", "AA001", 60, 2)
	if aaOk then print("Disciplina 'Algoritmos Avancados' (ID 4, prof 2)") end

	-- Disciplina 5: Calculo I (prof 3)
	local cal1Ok = dm:createSubject("Calculo I", "MAT101", 60, 3)
	if cal1Ok then print("Disciplina 'Calculo I' (ID 5, prof 3)") end

	-- Disciplina 6: Algebra Linear (prof 3)
	local algOk = dm:createSubject("Algebra Linear", "MAT201", 60, 3)
	if algOk then print("Disciplina 'Algebra Linear' (ID 6, prof 3)") end

	-- ============================================================
	-- MATRICULAS COM NOTAS (Aluno 1 - Joao)
	-- ============================================================

	-- Joao em Banco de Dados (2025-2, nota 85 - aprovado)
	if s1 and bdOk then
		dm:enrollStudent(1, 1, 1, "2025-2")
		dm:updateGrade(1, 1, 85)
		print("Joao > BD (2025-2): 85")
	end

	-- Joao em Logica de Programacao (2025-2, nota 42 - reprovado)
	if s1 and lpOk then
		dm:enrollStudent(1, 2, 1, "2025-2")
		dm:updateGrade(1, 2, 42)
		print("Joao > LP (2025-2): 42")
	end

	-- Joao em Estruturas de Dados (2026-1, nota 78 - aprovado)
	if s1 and edOk then
		dm:enrollStudent(1, 3, 2, "2026-1")
		dm:updateGrade(1, 3, 78)
		print("Joao > ED (2026-1): 78")
	end

	-- Joao em Calculo I (2026-1, nota 90 - aprovado)
	if s1 and cal1Ok then
		dm:enrollStudent(1, 5, 3, "2026-1")
		dm:updateGrade(1, 5, 90)
		print("Joao > Calc I (2026-1): 90")
	end

	-- Joao em Algebra Linear (2026-1, sem nota)
	if s1 and algOk then
		dm:enrollStudent(1, 6, 3, "2026-1")
		print("Joao > Alg Lin (2026-1): sem nota")
	end

	-- ============================================================
	-- MATRICULAS (Aluno 2 - Maria)
	-- ============================================================

	-- Maria em Logica de Programacao (2025-1, nota 70)
	if s2 and lpOk then
		dm:enrollStudent(2, 2, 1, "2025-1")
		dm:updateGrade(2, 2, 70)
		print("Maria > LP (2025-1): 70")
	end

	-- Maria em Estruturas de Dados (2025-2, nota 88)
	if s2 and edOk then
		dm:enrollStudent(2, 3, 2, "2025-2")
		dm:updateGrade(2, 3, 88)
		print("Maria > ED (2025-2): 88")
	end

	-- Maria em Algoritmos Avancados (2026-1, nota 55 - reprovado)
	if s2 and aaOk then
		dm:enrollStudent(2, 4, 2, "2026-1")
		dm:updateGrade(2, 4, 55)
		print("Maria > AA (2026-1): 55")
	end

	-- Maria em Algebra Linear (2026-1, sem nota)
	if s2 and algOk then
		dm:enrollStudent(2, 6, 3, "2026-1")
		print("Maria > Alg Lin (2026-1): sem nota")
	end

	-- ============================================================
	-- MATRICULAS (Aluno 3 - Pedro)
	-- ============================================================

	-- Pedro em Logica de Programacao (2024-1, nota 60 - aprovado limite)
	if s3 and lpOk then
		dm:enrollStudent(3, 2, 1, "2024-1")
		dm:updateGrade(3, 2, 60)
		print("Pedro > LP (2024-1): 60")
	end

	-- Pedro em Calculo I (2024-1, nota 95)
	if s3 and cal1Ok then
		dm:enrollStudent(3, 5, 3, "2024-1")
		dm:updateGrade(3, 5, 95)
		print("Pedro > Calc I (2024-1): 95")
	end

	-- Pedro em Estruturas de Dados (2024-2, nota 30 - reprovado)
	if s3 and edOk then
		dm:enrollStudent(3, 3, 2, "2024-2")
		dm:updateGrade(3, 3, 30)
		print("Pedro > ED (2024-2): 30")
	end

	-- Pedro em Algebra Linear (2025-1, sem nota)
	if s3 and algOk then
		dm:enrollStudent(3, 6, 3, "2025-1")
		print("Pedro > Alg Lin (2025-1): sem nota")
	end

	-- Pedro em Banco de Dados (2026-1, sem nota)
	if s3 and bdOk then
		dm:enrollStudent(3, 1, 1, "2026-1")
		print("Pedro > BD (2026-1): sem nota")
	end

	-- ============================================================
	-- MATRICULAS (Aluno 4 - Ana)
	-- ============================================================

	-- Ana em Algoritmos Avancados (2026-1, nota 82)
	if s4 and aaOk then
		dm:enrollStudent(4, 4, 2, "2026-1")
		dm:updateGrade(4, 4, 82)
		print("Ana > AA (2026-1): 82")
	end

	-- Ana em Calculo I (2026-1, nota 73)
	if s4 and cal1Ok then
		dm:enrollStudent(4, 5, 3, "2026-1")
		dm:updateGrade(4, 5, 73)
		print("Ana > Calc I (2026-1): 73")
	end

	-- Ana em Banco de Dados (2026-1, sem nota)
	if s4 and bdOk then
		dm:enrollStudent(4, 1, 1, "2026-1")
		print("Ana > BD (2026-1): sem nota")
	end

	-- ============================================================
	-- MATRICULAS (Aluno 5 - Lucas)
	-- ============================================================

	-- Lucas em Logica de Programacao (2024-1, nota 65)
	if s5 and lpOk then
		dm:enrollStudent(5, 2, 1, "2024-1")
		dm:updateGrade(5, 2, 65)
		print("Lucas > LP (2024-1): 65")
	end

	-- Lucas em Banco de Dados (2024-2, nota 91)
	if s5 and bdOk then
		dm:enrollStudent(5, 1, 1, "2024-2")
		dm:updateGrade(5, 1, 91)
		print("Lucas > BD (2024-2): 91")
	end

	-- Lucas em Estruturas de Dados (2025-1, nota 48 - reprovado)
	if s5 and edOk then
		dm:enrollStudent(5, 3, 2, "2025-1")
		dm:updateGrade(5, 3, 48)
		print("Lucas > ED (2025-1): 48")
	end

	-- Lucas em Estruturas de Dados novamente (2026-1, sem nota - repetente)
	if s5 and edOk then
		dm:enrollStudent(5, 3, 2, "2026-1")
		print("Lucas > ED (2026-1): sem nota (repetente)")
	end

	print("=== Populacao de dados concluida! ===")
end

return M
