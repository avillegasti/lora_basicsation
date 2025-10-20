#ifndef DB_H
#define DB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sqlite3.h>

/**
 * @brief Crea la base de datos y la tabla loraData si no existe.
 * 
 * @param db_name Nombre del archivo de la base de datos SQLite.
 * @return int SQLITE_OK si tuvo éxito, otro código de error SQLite en caso contrario.
 */
int create_db(const char *db_name);
int create_db_persistant(const char *db_name) ;
/**
 * @brief Inserta un payload binario en la tabla loraData.
 * 
 * @param db_name Nombre del archivo de la base de datos SQLite.
 * @param data Puntero al buffer de datos binarios.
 * @param data_len Tamaño en bytes del buffer.
 * @return int SQLITE_OK si tuvo éxito, otro código de error SQLite en caso contrario.
 */
int store_data(const char *db_name, const void *data, int data_len);

int create_db_saf(const char *db_name) ;
/**
 * @brief Lee todos los registros de la tabla loraData e imprime su contenido en hexadecimal.
 * 
 * @param db_name Nombre del archivo de la base de datos SQLite.
 * @return int SQLITE_OK si tuvo éxito, otro código de error SQLite en caso contrario.
 */
int read_data(const char *db_name);
int store_pkt_rx(const char *db_name, struct lgw_pkt_rx_s *pkt);
#endif /* LORA_DB_H */
