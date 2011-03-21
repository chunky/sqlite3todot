PRAGMA foreign_keys=ON;
BEGIN;

-- You can group tables into any number of "clusters"
--   For example, "structure" and "user content", or "input" and "output"
--   At time of writing, the color is the outline for the box
CREATE TABLE IF NOT EXISTS cluster (
	clusterid INTEGER PRIMARY KEY,
	label TEXT,
	color TEXT
);

-- Put each table that needs to go into a cluster into here
--   Tables not mentioned here are simply not added to a cluster
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


-- These are simply put in as-is as graph-wide attributes
CREATE TABLE IF NOT EXISTS graphsettings (
	setting TEXT NOT NULL
);

INSERT INTO graphsettings (setting) VALUES ("rankdir=LR");
INSERT INTO graphsettings (setting) VALUES ("splines=true");
INSERT INTO graphsettings (setting) VALUES ("overlap=false");


-- Tables mentioned here aren't included in the output
CREATE TABLE IF NOT EXISTS ignorelist (
	tbl_name TEXT NOT NULL
);

-- These are created by ANALYZE
INSERT INTO ignorelist (tbl_name) VALUES ("sqlite_stat1");
INSERT INTO ignorelist (tbl_name) VALUES ("sqlite_stat2");

COMMIT;

