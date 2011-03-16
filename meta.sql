PRAGMA foreign_keys=ON;
BEGIN;

CREATE TABLE IF NOT EXISTS cluster (
	clusterid INTEGER PRIMARY KEY,
	label TEXT,
	color TEXT
);

CREATE TABLE IF NOT EXISTS tbl_cluster (
	tbl_name TEXT NOT NULL,
	clusterid INTEGER NOT NULL,
	FOREIGN KEY(clusterid) REFERENCES cluster(clusterid)
);

INSERT INTO cluster (label,color) VALUES ("Blue Stuff", "blue");
INSERT INTO cluster (label,color) VALUES ("Red Stuff", "red");

INSERT INTO tbl_cluster (tbl_name, clusterid) VALUES ("tbl_cluster", (SELECT clusterid FROM cluster WHERE label="Blue Stuff"));
INSERT INTO tbl_cluster (tbl_name, clusterid) VALUES ("cluster", (SELECT clusterid FROM cluster WHERE label="Blue Stuff"));

INSERT INTO tbl_cluster (tbl_name, clusterid) VALUES ("ignorelist", (SELECT clusterid FROM cluster WHERE label="Red Stuff"));


CREATE TABLE IF NOT EXISTS graphsettings (
	setting TEXT NOT NULL
);

INSERT INTO graphsettings (setting) VALUES ("rankdir=LR");
INSERT INTO graphsettings (setting) VALUES ("splines=true");
INSERT INTO graphsettings (setting) VALUES ("overlap=false");


CREATE TABLE IF NOT EXISTS ignorelist (
	tbl_name TEXT NOT NULL
);

INSERT INTO ignorelist (tbl_name) VALUES ("sqlite_stat1");
INSERT INTO ignorelist (tbl_name) VALUES ("sqlite_stat2");

COMMIT;

