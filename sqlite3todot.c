/**
sqlite3todot, originally by Gary "ChunkyKs" Briggs <chunky@icculus.org>

The license for this code is the same as that of sqlite

May you do good and not evil
May you find forgiveness for yourself and forgive others
May you share freely, never taking more than you give.


 gcc -o sqlite3todot -lsqlite3 sqlite3todot.c
 ./sqlite3todot {dbname} [metadata db] | dot -Tpng -osomething.png

*/

#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

int main(int argc, char **argv) {
	if(argc < 2 || 0 == strcmp(argv[1],"--help") || 0 == strcmp(argv[1], "-h")) {
		fprintf(stderr, "Usage: %s <dbname> [metadb]\n", argv[0]);
		exit(1);
	}

	sqlite3 *db = NULL;
	int rc;
	char *dbname = argv[1];
	rc = sqlite3_open_v2(dbname, &db, SQLITE_OPEN_READONLY, NULL);
	if(SQLITE_OK != rc) {
		fprintf(stderr, "Can't open database %s (%i): %s\n", dbname, rc, sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	int have_meta = 0;
	if(argc > 2) {
		char attach_sql[1024];
		char *exec_errmsg;
		snprintf(attach_sql,sizeof(attach_sql),"ATTACH \"%s\" AS meta", argv[2]);
		rc = sqlite3_exec(db, attach_sql, NULL, NULL, &exec_errmsg);
		if(SQLITE_OK != rc) {
			fprintf(stderr, "Error attaching meta db (%i): %s\n", rc, exec_errmsg);
			sqlite3_free(exec_errmsg);
			sqlite3_close(db);
			exit(1);
		}
		have_meta = 1;
	}

	const char *tbl_list_sql = "SELECT tbl_name,NULL AS label,NULL AS color,NULL AS clusterid "
		"FROM sqlite_master WHERE type='table'";
	if(have_meta) {
		tbl_list_sql = "SELECT sqlite_master.tbl_name AS tbl_name, meta.cluster.label AS label, meta.cluster.color AS color, meta.cluster.clusterid AS clusterid \n"
			"FROM sqlite_master \n"
			"LEFT JOIN meta.tbl_cluster ON sqlite_master.tbl_name=meta.tbl_cluster.tbl_name \n"
			"LEFT JOIN meta.cluster ON meta.tbl_cluster.clusterid=meta.cluster.clusterid \n"
			"LEFT JOIN meta.ignorelist ON sqlite_master.tbl_name=meta.ignorelist.tbl_name \n"
			"WHERE meta.ignorelist.tbl_name IS NULL \n"
			"       AND main.sqlite_master.type='table'\n"
			"GROUP BY sqlite_master.tbl_name\n"
			"ORDER BY meta.cluster.clusterid\n";
	}
	sqlite3_stmt *tbl_list_stmt;

	// fprintf(stderr, "%s\n", tbl_list_sql);

	rc = sqlite3_prepare_v2(db, tbl_list_sql, -1, &tbl_list_stmt, NULL);
	if(SQLITE_OK != rc) {
		fprintf(stderr, "Can't prepare table list statement (%i): %s\n", rc, sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	printf("digraph sqliteschema {\n", dbname);
	printf("node [shape=plaintext];\n");

	if(have_meta) {
		const char settings_sql[] = "SELECT setting FROM meta.graphsettings";
		sqlite3_stmt *settings_stmt;
		rc = sqlite3_prepare_v2(db, settings_sql, -1, &settings_stmt, NULL);
		if(SQLITE_OK != rc) {
			fprintf(stderr, "Warning: Cannot find meta.graphsettings (%i): %s\n", rc, sqlite3_errmsg(db));
		} else {
			while(SQLITE_ROW == (rc = sqlite3_step(settings_stmt))) {
				printf("%s\n", sqlite3_column_text(settings_stmt,0));
			}
			sqlite3_finalize(settings_stmt);
		}
	} else {
		printf("rankdir=LR\n");
		printf("splines=true\n");
		printf("overlap=scale\n");
	}

	const int cols = 4;

	int curr_cluster = -1;
	while(SQLITE_ROW == (rc = sqlite3_step(tbl_list_stmt))) {
		int i;
		const char *tbl_name = sqlite3_column_text(tbl_list_stmt, 0);

		int cluster_id = SQLITE_NULL==sqlite3_column_type(tbl_list_stmt,3)?-1:sqlite3_column_int(tbl_list_stmt,3);
		if(cluster_id != curr_cluster && curr_cluster != -1) {
			printf("}\n");
		}
		if(cluster_id != curr_cluster && cluster_id != -1) {
			printf("subgraph cluster_%i {\n", cluster_id);
			if(SQLITE_NULL!=sqlite3_column_type(tbl_list_stmt,1)) {
				printf("label=\"%s\"\n", sqlite3_column_text(tbl_list_stmt,1));
			}
			if(SQLITE_NULL!=sqlite3_column_type(tbl_list_stmt,2)) {
				printf("color=\"%s\"\n", sqlite3_column_text(tbl_list_stmt,2));
			}
		}
		curr_cluster=cluster_id;

		char *tbl_info_sql = sqlite3_mprintf("PRAGMA table_info(%q)", tbl_name);
		sqlite3_stmt *tbl_info_stmt;

		rc = sqlite3_prepare_v2(db, tbl_info_sql, -1, &tbl_info_stmt, NULL);
		if(SQLITE_OK != rc) {
			fprintf(stderr, "Can't prepare table info statement on table %s (%i): %s\n", tbl_name, rc, sqlite3_errmsg(db));
			sqlite3_close(db);
			exit(1);
		}

		printf("%s [label=<<TABLE CELLSPACING=\"0\"><TR><TD COLSPAN=\"%i\"><U>%s</U></TD></TR>", tbl_name, cols, tbl_name);
		int curr_row = 0;
		int in_brace = 0;
		while(SQLITE_ROW == (rc = sqlite3_step(tbl_info_stmt))) {
			if(0 == curr_row%cols) {
				in_brace = 1;
				printf("<TR>");
			}
			printf("<TD PORT=\"%s\">%s</TD>",
				sqlite3_column_text(tbl_info_stmt, 1),
				sqlite3_column_text(tbl_info_stmt, 1));
			curr_row++;
			if(0 == curr_row%cols) {
				in_brace = 0;
				printf("</TR>");
			}
		}
		if(in_brace) {
			printf("</TR>");
		}
		printf("</TABLE>>];\n");

		sqlite3_free(tbl_info_sql);
		sqlite3_finalize(tbl_info_stmt);

	}
	if(curr_cluster != -1) {
		printf("}\n");
	}

	sqlite3_reset(tbl_list_stmt);
	while(SQLITE_ROW == (rc = sqlite3_step(tbl_list_stmt))) {
		const char *tbl_name = sqlite3_column_text(tbl_list_stmt, 0);

		char *fkey_info_sql = sqlite3_mprintf("PRAGMA foreign_key_list(%q)", tbl_name);
		sqlite3_stmt *fkey_info_stmt;

		rc = sqlite3_prepare_v2(db, fkey_info_sql, -1, &fkey_info_stmt, NULL);
		if(SQLITE_OK != rc) {
			fprintf(stderr, "Can't prepare foreign key statement on table %s (%i): %s\n", tbl_name, rc, sqlite3_errmsg(db));
			sqlite3_close(db);
			exit(1);
		}

		while(SQLITE_ROW == (rc = sqlite3_step(fkey_info_stmt))) {
			printf("%s:%s -> %s:%s;\n",
				tbl_name,
				sqlite3_column_text(fkey_info_stmt, 3),
				sqlite3_column_text(fkey_info_stmt, 2),
				sqlite3_column_text(fkey_info_stmt, 4));
		}

		sqlite3_free(fkey_info_sql);
		sqlite3_finalize(fkey_info_stmt);
	}

	printf("}\n");

	sqlite3_finalize(tbl_list_stmt);
	sqlite3_close(db);
}

