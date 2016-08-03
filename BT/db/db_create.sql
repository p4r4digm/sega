--
-- File generated with SQLiteStudio v3.1.0 on Tue Aug 2 20:43:25 2016
--
-- Text encoding used: System
--
PRAGMA foreign_keys = off;
BEGIN TRANSACTION;

-- Table: Images
CREATE TABLE Images (id STRING PRIMARY KEY NOT NULL UNIQUE ON CONFLICT REPLACE, image BLOB);

-- Table: Palettes
CREATE TABLE Palettes (id STRING PRIMARY KEY NOT NULL UNIQUE ON CONFLICT REPLACE, palette BLOB);

COMMIT TRANSACTION;
PRAGMA foreign_keys = on;