/**
 * @file DataManager.hpp
 * @brief Controlador principal - CRUD exposto para Lua.
 * @namespace project_controller
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include "FileManager.hpp"
#include "IndexCtrl.hpp"
#include "../utility/Constants.hpp"
#include "../model/Record.hpp"

namespace project_controller {

	/**
	 * @brief Controlador principal - integra FileManager e IndexCtrl.
	 * 
	 * Expoe operacoes CRUD para binding com Lua.
	 * Gerencia IDs dinamicos (recalculados a cada listagem).
	 * Soft delete com marcacao '*' no arquivo.
	 */
	class DataManager {
	public:
		DataManager() = default;
		~DataManager() = default;

		/**
		 * @brief Inicializa arquivos de dados e indice.
		 * @param dataPath Caminho do arquivo .dat
		 * @return true se bem-sucedido
		 */
		[[nodiscard]] bool initialize(const std::string& dataPath);

		/**
		 * @brief Cria novo estudante.
		 * @param name Nome do estudante
		 * @param userId ID do usuario
		 * @param birthDate Data de nascimento (YYYYMMDD)
		 * @return true se bem-sucedido
		 */
		[[nodiscard]] bool createStudent(const std::string& name, int32_t userId, uint32_t birthDate);

		/**
		 * @brief Le estudante por ID dinamico.
		 * @param id ID dinamico (1-based, recalculado)
		 * @return StudentRecord ou nullopt se nao encontrado
		 */
		[[nodiscard]] std::optional<project_model::StudentRecord> readStudent(int32_t id) const;

		/**
		 * @brief Busca estudante por nome via indice hash.
		 * @param name Nome a buscar
		 * @return StudentRecord ou nullopt se nao encontrado
		 */
		[[nodiscard]] std::optional<project_model::StudentRecord> searchByName(const std::string& name) const;

		/**
		 * @brief Deleta estudante (soft delete com '*').
		 * @param id ID dinamico
		 * @return true se bem-sucedido
		 */
		[[nodiscard]] bool deleteStudent(int32_t id);

		/**
		 * @brief Lista todos estudantes ativos com IDs recalculados.
		 * @return Vetor de StudentRecord (IDs 1, 2, 3, ...)
		 */
		[[nodiscard]] std::vector<project_model::StudentRecord> listAll() const;

		/**
		 * @brief Retorna ultima mensagem de erro.
		 * @return String de erro
		 */
		[[nodiscard]] std::string getLastError() const;

		/**
		 * @brief Verifica se precisa reconstruir indice.
		 * @return true se multiplo de 10 registros ativos
		 */
		[[nodiscard]] bool needsRebuild() const;

		/**
		 * @brief Trigger reconstrucao do indice.
		 */
		void triggerRebuild();

		/**
		 * @brief Ignora reconstrucao para sessao atual.
		 */
		void ignoreRebuildForSession();

		/**
		 * @brief Proximo ID dinamico (para exibicao).
		 * @return ID para novo registro
		 */
		[[nodiscard]] int32_t getNextDisplayId() const;

		/**
		 * @brief Conta registros ativos.
		 * @return Numero de registros com status 'A'
		 */
		[[nodiscard]] int32_t getActiveCount() const;

	private:
		/**
		 * @brief Reconstrucao completa do indice a partir do arquivo de dados.
		 */
		void rebuildIndex();

		/**
		 * @brief Escaneia registros ativos e retorna pares (offset, record).
		 * @return Vetor de pares (offset, StudentRecord)
		 */
		[[nodiscard]] std::vector<std::pair<size_t, project_model::StudentRecord>> scanActive() const;

		FileManager fileMgr_;
		IndexCtrl indexCtrl_;
		std::string dataPath_;
		mutable std::string lastError_;
		bool rebuildIgnored_ = false;
	};

} // namespace project_controller