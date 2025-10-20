#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sqlite3.h>
#include "lgw/loragw_hal.h"
#include "xq.h"

/* Create database and table */
int create_db(const char *db_name) {
    sqlite3 *db;
    char *errMsg = 0;
    int rc = sqlite3_open(db_name, &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    const char *create_sql =
        "CREATE TABLE IF NOT EXISTS loraData ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "payload BLOB NOT NULL);";

    rc = sqlite3_exec(db, create_sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    sqlite3_close(db);
    return rc;
}

int create_db_saf(const char *db_name) {
    sqlite3 *db;
    char *errMsg = 0;
    int rc = sqlite3_open(db_name, &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    const char *create_sql =
        "CREATE TABLE IF NOT EXISTS loraDataRXJOB ("
           " id          INTEGER PRIMARY KEY AUTOINCREMENT,"
           " freq_hz     INTEGER,"
            "freq_offset INTEGER,"
            "if_chain    INTEGER,"
            "status      INTEGER,"
            "count_us    INTEGER,"
            "rf_chain    INTEGER,"
            "modem_id    INTEGER,"
            "modulation  INTEGER,"
            "bandwidth   INTEGER,"
            "datarate    INTEGER,"
            "coderate    INTEGER,"
            "rssic       REAL,"
            "rssis       REAL,"
            "snr         REAL,"
            "snr_min     REAL,"
            "snr_max     REAL,"
            "crc         INTEGER,"
            "size        INTEGER,"
            "payload     BLOB,"
            "ftime_received INTEGER,"
            "ftime       INTEGER"
        ");";

    rc = sqlite3_exec(db, create_sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    sqlite3_close(db);
    return rc;
}

int create_db_persistant(const char *db_name) {
    sqlite3 *db;
    char *errMsg = 0;
    int rc = sqlite3_open(db_name, &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    const char *create_sql =
    "CREATE TABLE IF NOT EXISTS lora_persistant ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "freq_hz INTEGER, "
        "dr INTEGER, "
        "rssi INTEGER, "
        "snr REAL, "
        "xtime INTEGER, "
        "fts INTEGER, "
        "crc INTEGER, "
        "len INTEGER, "
        "payload BLOB, "
        "saved_at DATETIME DEFAULT CURRENT_TIMESTAMP);";

    rc = sqlite3_exec(db, create_sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    sqlite3_close(db);
    return rc;
}




/* Store binary payload into database */
int store_data(const char *db_name, const void *data, int data_len) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc = sqlite3_open(db_name, &db);
    
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    const char *sql = "INSERT INTO loraData (payload) VALUES(?);";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // Bind payload as raw BLOB
    rc = sqlite3_bind_blob(stmt, 1, data, data_len, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Bind failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return rc;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    printf("Data stored in database  %s\n", db_name);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

/* Read and print stored payloads */
int read_data(const char *db_name) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc = sqlite3_open(db_name, &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    const char *sql = "SELECT id, payload FROM loraData;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const void *blob = sqlite3_column_blob(stmt, 1);
        int blob_size = sqlite3_column_bytes(stmt, 1);

        printf("Row %d: payload (%d bytes) = ", id, blob_size);

        const unsigned char *bytes = (const unsigned char *)blob;
        for (int i = 0; i < blob_size; i++) {
            printf("%02X ", bytes[i]);
        }
        printf("\n");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return SQLITE_OK;
}

int store_pkt_rx(const char *db_name, struct lgw_pkt_rx_s *pkt) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc = sqlite3_open(db_name, &db);
    if (rc) { fprintf(stderr,"Can't open db: %s\n", sqlite3_errmsg(db)); return rc; }

    const char *sql =
        "INSERT INTO loraDataRxJOB (freq_hz,freq_offset,if_chain,status,count_us,rf_chain,"
        "modem_id,modulation,bandwidth,datarate,coderate,rssic,rssis,snr,snr_min,snr_max,"
        "crc,size,payload,ftime_received,ftime) "
        "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if(rc!=SQLITE_OK){fprintf(stderr,"Prepare failed: %s\n", sqlite3_errmsg(db)); sqlite3_close(db); return rc;}

    sqlite3_bind_int(stmt, 1, pkt->freq_hz);
    sqlite3_bind_int(stmt, 2, pkt->freq_offset);
    sqlite3_bind_int(stmt, 3, pkt->if_chain);
    sqlite3_bind_int(stmt, 4, pkt->status);
    sqlite3_bind_int(stmt, 5, pkt->count_us);
    sqlite3_bind_int(stmt, 6, pkt->rf_chain);
    sqlite3_bind_int(stmt, 7, pkt->modem_id);
    sqlite3_bind_int(stmt, 8, pkt->modulation);
    sqlite3_bind_int(stmt, 9, pkt->bandwidth);
    sqlite3_bind_int(stmt,10, pkt->datarate);
    sqlite3_bind_int(stmt,11, pkt->coderate);
    sqlite3_bind_double(stmt,12, pkt->rssic);
    sqlite3_bind_double(stmt,13, pkt->rssis);
    sqlite3_bind_double(stmt,14, pkt->snr);
    sqlite3_bind_double(stmt,15, pkt->snr_min);
    sqlite3_bind_double(stmt,16, pkt->snr_max);
    sqlite3_bind_int(stmt,17, pkt->crc);
    sqlite3_bind_int(stmt,18, pkt->size);
    sqlite3_bind_blob(stmt,19, pkt->payload, pkt->size, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,20, pkt->ftime_received ? 1 : 0);
    sqlite3_bind_int(stmt,21, pkt->ftime);

    rc = sqlite3_step(stmt);
    if(rc!=SQLITE_DONE) fprintf(stderr,"Insert failed: %s\n",sqlite3_errmsg(db));

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rc==SQLITE_DONE ? SQLITE_OK : rc;
}


/* Store rxjob in persistent database */
int store_rxjob_persist(const char *db_name, rxjob_t *rxjob, const u1_t *rxdata, uint16_t crc) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc = sqlite3_open(db_name, &db);
    if (rc) {
        fprintf(stderr, "Can't open DB %s: %s\n", db_name, sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    const char *sql =
        "INSERT INTO lora_persistant (freq_hz, dr, rssi, snr, xtime, fts, crc, len, payload, saved_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP);";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    sqlite3_bind_int(stmt, 1, rxjob->freq);
    sqlite3_bind_int(stmt, 2, rxjob->dr);
    sqlite3_bind_int(stmt, 3, rxjob->rssi);
    sqlite3_bind_double(stmt, 4, rxjob->snr / 4.0);   // scaled back to float
    sqlite3_bind_int64(stmt, 5, rxjob->xtime);
    sqlite3_bind_int(stmt, 6, rxjob->fts);
    sqlite3_bind_int(stmt, 7, crc);
    sqlite3_bind_int(stmt, 8, rxjob->len);
    sqlite3_bind_blob(stmt, 9, &rxdata[rxjob->off], rxjob->len, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Stored rxjob: freq=%u, len=%u, dr=%u\n", rxjob->freq, rxjob->len, rxjob->dr);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}
