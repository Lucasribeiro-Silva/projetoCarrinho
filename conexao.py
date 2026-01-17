import mysql.connector

def conectar_db():

    # ---- CONECTA NO BANCO  ----
    db = mysql.connector.connect(
        host="----",
        user="----",
        password="----",
        database="----"
    )
    cursor = db.cursor()

    # ---- CRIA A TABELA SE NÃO EXISTIR ----
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS sensores (
            id_sensor INT PRIMARY KEY,
            rpm INT DEFAULT 0,
            vel INT DEFAULT 0,
            ldr INT DEFAULT 0,
            dist INT DEFAULT 0,
            farol BOOLEAN DEFAULT 0,
            buzina BOOLEAN DEFAULT 0
        )
    """)
    db.commit()

    # ---- GARANTE QUE EXISTA A LINHA id = 1 ----
    cursor.execute("SELECT COUNT(*) FROM sensores WHERE id_sensor = 1")
    existe = cursor.fetchone()[0]

    if existe == 0:
        cursor.execute("""
            INSERT INTO sensores (id_sensor, rpm, vel, ldr, dist, farol, buzina)
            VALUES (1, 0, 0, 0, 0, 0, 0)
        """)
        db.commit()

    return db, cursor
