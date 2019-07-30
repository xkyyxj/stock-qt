
insert into ana_category(category_name, pk_parent, pk_tablemeta) values('常用类型', null, null);

insert into ana_category(category_name, pk_parent, pk_tablemeta) values('V型反转', '1', '2');

insert into ana_category(category_name, pk_parent, pk_tablemeta) values('快速上涨', '1', '1');

insert into table_meta(table_name) values('quick_up');

insert into table_column(display_name, columntype, pk_tablemeta, column_name)  values('编码','string','1','ts_code');

insert into table_column(display_name, columntype, pk_tablemeta, column_name)  values('名称','string','1','ts_name');
;
insert into table_column(display_name, columntype, pk_tablemeta, column_name)  values('涨幅','double','1','up_pｃt');

insert into table_column(display_name, columntype, pk_tablemeta, column_name)  values('日期','date','1','date');

insert into table_meta(table_name) values('v_wave');

insert into table_column(display_name, columntype, pk_tablemeta, column_name)  values('编码','string','2','ts_code');

insert into table_column(display_name, columntype, pk_tablemeta, column_name)  values('名称','string','2','ts_name');

insert into table_column(display_name, columntype, pk_tablemeta, column_name)  values('日期','date','2','date');

insert into table_meta(table_name) values('big_wave');

insert into table_column(display_name, columntype, pk_tablemeta, column_name)  values('编码','string','3','ts_code');

insert into table_column(display_name, columntype, pk_tablemeta, column_name)  values('名称','string','3','ts_name');

insert into table_column(display_name, columntype, pk_tablemeta, column_name)  values('日期','date','3','date');

insert into table_column(display_name, columntype, pk_tablemeta, column_name)  values('平均差','float','3','stddev');

insert into table_column(display_name, columntype, pk_tablemeta, column_name)  values('均值','float','3','ave');
