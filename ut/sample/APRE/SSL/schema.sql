DROP TABLE GOODS;
CREATE TABLE GOODS 
    ( 
        GNO            CHAR(10)     PRIMARY KEY, 
        GNAME          CHAR(20)     UNIQUE NOT NULL, 
        GOODS_LOCATION CHAR(9),
        STOCK          INTEGER      DEFAULT 0, 
        PRICE          NUMERIC(10,2) 
    );

