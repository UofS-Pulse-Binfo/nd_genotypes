
Example Database
================

The following queries endeavour to show how data used by this module is stored. This is a small peak into a production database and while it's not perfect (still containing some legacy terms, etc.) it is completely functional with the nd_genotypes module.

Markers & Variants
------------------

The following queries show how markers and variants are stored. The types used for markers and variants can be configured and more then one type can be used for each (e.g. you could use SNP, MNP, Indel types for variants). While the example below shows multiple types for variants, in the future my personal database will be switched to use the SO sequence_variant type for all variants to aid with consistent variant pages in Tripal 3. However, this is a personal choice and both methods have their pro's and cons.

.. code-block:: console

  psql=# SELECT f.*, cvt.name as type_name FROM chado.feature f LEFT JOIN chado.cvterm cvt ON cvt.cvterm_id=f.type_id WHERE f.name~'LcC09269p298';
   feature_id | dbxref_id | organism_id |                 name                  |          uniquename           | residues | seqlen | md5checksum | type_id | is_analysis | is_obsolete |      timeaccessioned       |      timelastmodified      |   type_name
  ------------+-----------+-------------+---------------------------------------+-------------------------------+----------+--------+-------------+---------+-------------+-------------+----------------------------+----------------------------+----------------
       327991 |   2513464 |           4 | LcC09269p298                          | LcC09269p298                  |          |      1 |             |     796 | f           | f           | 2011-07-29 16:08:43.515889 | 2011-07-29 16:08:43.515889 | SNP
       372934 |   2649322 |           4 | LcC09269p298 454 Sequencing           | LcC09269p298_454              |          |      1 |             |    3969 | f           | f           | 2011-09-15 11:52:45.943205 | 2011-09-15 11:52:45.943205 | genetic_marker
       392501 |   3114923 |           4 | LcC09269p298 Lc1536 Golden Gate Assay | LcC09269p298-1_B_F_1890446698 |          |      1 |             |    3969 | f           | f           | 2011-09-15 12:06:20.86547  | 2011-09-15 12:06:20.86547  | genetic_marker
  (3 rows)


.. code-block:: console

  psql=# SELECT prop.*, cvt.name as type_name FROM chado.featureprop prop LEFT JOIN chado.cvterm cvt ON cvt.cvterm_id=prop.type_id WHERE prop.feature_id IN (327991, 372934, 392501);
   featureprop_id | feature_id | type_id |           value            | rank |          type_name
  ----------------+------------+---------+----------------------------+------+-----------------------------
           400633 |     327991 |    1512 | 91 bp                      |    0 | five_prime_flanking_region
           400634 |     327991 |    1513 | 308 bp                     |    0 | three_prime_flanking_region
           525105 |     372934 |    3966 | 454 Sequencing             |    0 | marker_type
           459336 |     392501 |    1891 | 0.909                      |    0 | score
           459337 |     392501 |    1870 | LcRedberry                 |    0 | source
           459338 |     392501 |    3687 | 12/23/2010                 |    0 | design_date
           466357 |     392501 |    3709 | BOT                        |    0 | illumina_strand
           466358 |     392501 |    3710 | BOT                        |    0 | reference_sequence_strand
           781915 |     392501 |    3966 | Illumina Golden Gate Assay |    0 | marker_type
  (9 rows)


.. code-block:: console

  psql=# SELECT t.* FROM chado.featureloc t WHERE t.feature_id IN (327991, 372934, 392501);
   featureloc_id | feature_id | srcfeature_id |   fmin    | is_fmin_partial |   fmax    | is_fmax_partial | strand | phase | residue_info | locgroup | rank
  ---------------+------------+---------------+-----------+-----------------+-----------+-----------------+--------+-------+--------------+----------+------
         3897843 |     372934 |        295264 |       297 | f               |       298 | f               |      0 |     0 |              |        0 |    0
         3711470 |     392501 |        295264 |       297 | f               |       298 | f               |      0 |     0 |              |        0 |    0
         3260896 |     327991 |        295264 |       297 | f               |       298 | f               |        |       |              |        0 |    0
         4562009 |     327991 |       3400411 | 250519947 | f               | 250519948 | f               |     -1 |       |              |        2 |    0
         4562010 |     327991 |       3400411 | 250136623 | f               | 250136624 | f               |     -1 |       |              |        2 |    1
         4562011 |     327991 |       3400407 |    501710 | f               |    501711 | f               |     -1 |       |              |        2 |    2
         4628689 |     372934 |       3400411 | 250519947 | f               | 250519948 | f               |     -1 |       |              |        2 |    0
         4628690 |     372934 |       3400411 | 250136623 | f               | 250136624 | f               |     -1 |       |              |        2 |    1
         4628691 |     372934 |       3400407 |    501710 | f               |    501711 | f               |     -1 |       |              |        2 |    2
  (9 rows)


.. code-block:: console

  psql=# SELECT t.*, cvt.name as type_name FROM chado.feature_relationship t LEFT JOIN chado.cvterm cvt ON cvt.cvterm_id=t.type_id WHERE t.subject_id IN (327991, 372934, 392501);
   feature_relationship_id | subject_id | object_id | type_id | value | rank |  type_name
  -------------------------+------------+-----------+---------+-------+------+--------------
                   2575387 |     372934 |    327991 |    3685 |       |    0 | is_marker_of
                   2594954 |     392501 |    327991 |    3685 |       |    0 | is_marker_of
  (2 rows)


Genotypes
---------

The preferred method of storing genotype calls is to use the new genotype_call table created by this module as it is more efficient. As you can see below this results in each unique allele only being stored once in the genotype table with the information of which allele was detected for a given marker/stock combination is recorded in the genotype_call table. This method doesn't use the feature_genotype table.

.. code-block:: console

  psql=# SELECT t.*, cvt.name as type_name FROM chado.feature_genotype t LEFT JOIN chado.cvterm cvt ON cvt.cvterm_id=t.cvterm_id WHERE t.feature_id IN (327991, 372934, 392501);
   feature_genotype_id | feature_id | genotype_id | chromosome_id | rank | cgroup | cvterm_id | type_name
  ---------------------+------------+-------------+---------------+------+--------+-----------+-----------
  (0 rows)


.. code-block:: console

  psql=# SELECT * FROM chado.genotype_call WHERE variant_id=327991 LIMIT 10;
   genotype_call_id | variant_id | marker_id | genotype_id | project_id | stock_id | meta_data
  ------------------+------------+-----------+-------------+------------+----------+-----------
             158529 |     327991 |    372934 |     2625650 |          3 |    27907 |
             158530 |     327991 |    372934 |     2625649 |          3 |    27908 |
             158531 |     327991 |    372934 |     2625649 |          3 |    27911 |
             324755 |     327991 |    372934 |     2625650 |          3 |    27916 |
             324756 |     327991 |    372934 |     2625650 |          3 |    27917 |
             616977 |     327991 |    392501 |     2625652 |         36 |    28283 |
             618223 |     327991 |    392501 |     2625652 |         36 |    28284 |
             619485 |     327991 |    392501 |     2625651 |         36 |    28285 |
             620644 |     327991 |    392501 |     2625651 |         36 |    28286 |
             621871 |     327991 |    392501 |     2625652 |         36 |    28287 |
  (10 rows)


.. code-block:: console

  psql=# SELECT g.*, cvt.name as type_name FROM chado.genotype g LEFT JOIN chado.cvterm cvt ON cvt.cvterm_id=g.type_id;
   genotype_id | name | uniquename | description | type_id | type_name
  -------------+------+------------+-------------+---------+-----------
       2625647 | A    | A          | A           |     796 | SNP
       2625648 | T    | T          | T           |     796 | SNP
       2625649 | C    | C          | C           |     796 | SNP
       2625650 | G    | G          | G           |     796 | SNP
       2625651 | GG   | GG         | GG          |     796 | SNP
       2625652 | CC   | CC         | CC          |     796 | SNP
       2625653 | TT   | TT         | TT          |     796 | SNP
       2625654 | AA   | AA         | AA          |     796 | SNP
  (8 rows)


Germplasm/Stocks
----------------

The DNA source the marker assay was performed on is given a type of DNA with the original germplasm source of this DNA having whichever term is appropriate. The important thing is that the DNA extraction and original germplasm are related consistently through the stock_relationship table.

.. code-block:: console

  psql=# SELECT s.*, cvt.name as type_name FROM chado.stock s LEFT JOIN chado.cvterm cvt ON cvt.cvterm_id=s.type_id WHERE s.stock_id IN (58, 27907);
    stock_id | dbxref_id | organism_id |                     name                      |               uniquename               | description | type_id | is_obsolete | type_name
  ----------+-----------+-------------+-----------------------------------------------+----------------------------------------+-------------+---------+-------------+------------
         58 |   1901662 |           4 | CDC Redberry                                  | KP:GERM58                              |             |    3683 | f           | Variety
      27907 |           |           4 | CDC Redberry 454 Extraction                   | CDC_Redberry_454                       |             |    3630 | f           | DNA


.. code-block:: console

  psql=# SELECT t.*, cvt.name as type_name FROM chado.stock_relationship t LEFT JOIN chado.cvterm cvt ON cvt.cvterm_id=t.type_id WHERE t.subject_id IN (58, 27907) AND cvt.name='is_extracted_from';
   stock_relationship_id | subject_id | object_id | type_id | value | rank |     type_name
  -----------------------+------------+-----------+---------+-------+------+-------------------
                   43301 |      27907 |        58 |    3712 |       |    0 | is_extracted_from
  (1 row)


Materialized Views
------------------

The following queries show the materialized views created by this module and provide an example of what they should contain. Notice that the variant/markers being demonstrated are located in multiple places on the genotype which explains the multiple records in mview_ndg_lens_variants. If your variants amplify unique regions then there will only be one location per variant in this table.


.. code-block:: console

  psql=# SELECT * FROM chado.mview_ndg_lens_calls WHERE variant_id=327991 LIMIT 10;
   variant_id | marker_id |              marker_name              |        marker_type         | stock_id |         stock_name          | germplasm_id | germplasm_name | project_id | genotype_id | allele_call | meta_data | ndg_call_id
  ------------+-----------+---------------------------------------+----------------------------+----------+-----------------------------+--------------+----------------+------------+-------------+-------------+-----------+-------------
       327991 |    372934 | LcC09269p298 454 Sequencing           | 454 Sequencing             |    27908 | 964a-46 454 Extraction      |         6755 | 964a-46        |          3 |     2625649 | C           |           |     1223711
       327991 |    372934 | LcC09269p298 454 Sequencing           | 454 Sequencing             |    27911 | ILL 8006 454 Extraction     |        18809 | ILL 8006       |          3 |     2625649 | C           |           |     1223712
       327991 |    372934 | LcC09269p298 454 Sequencing           | 454 Sequencing             |    27907 | CDC Redberry 454 Extraction |           58 | CDC Redberry   |          3 |     2625650 | G           |           |     1309137
       327991 |    372934 | LcC09269p298 454 Sequencing           | 454 Sequencing             |    27916 | PI 320937 454 Extraction    |         7832 | PI 320937      |          3 |     2625650 | G           |           |     1347692
       327991 |    372934 | LcC09269p298 454 Sequencing           | 454 Sequencing             |    27917 | L01-827A 454 Extraction     |         9727 | L01-827A       |          3 |     2625650 | G           |           |     1347693
       327991 |    392501 | LcC09269p298 Lc1536 Golden Gate Assay | Illumina Golden Gate Assay |    28285 | 1294M-23 Extraction         |         9420 | 1294M-23       |         36 |     2625651 | GG          |           |     1357149
       327991 |    392501 | LcC09269p298 Lc1536 Golden Gate Assay | Illumina Golden Gate Assay |    28286 | 2670B Extraction            |         9975 | 2670B          |         36 |     2625651 | GG          |           |     1357418
       327991 |    392501 | LcC09269p298 Lc1536 Golden Gate Assay | Illumina Golden Gate Assay |    28288 | 964a-46 Extraction          |         6755 | 964a-46        |         36 |     2625651 | GG          |           |     1357955
       327991 |    392501 | LcC09269p298 Lc1536 Golden Gate Assay | Illumina Golden Gate Assay |    28289 | Giftgi Extraction           |         9771 | Giftgi         |         36 |     2625651 | GG          |           |     1358196
       327991 |    392501 | LcC09269p298 Lc1536 Golden Gate Assay | Illumina Golden Gate Assay |    28290 | ILL 1704 Extraction         |         8111 | ILL 1704       |         36 |     2625651 | GG          |           |     1358495
  (10 rows)

  psql=# SELECT * FROM chado.mview_ndg_lens_variants WHERE variant_id=327991;
   variant_id | variant_name | variant_type | srcfeature_id | srcfeature_name |   fmin    |   fmax    |                             meta_data                              | ndg_variants_id
  ------------+--------------+--------------+---------------+-----------------+-----------+-----------+--------------------------------------------------------------------+-----------------
       327991 | LcC09269p298 | SNP          |        295264 | LcRBContig09269 |       297 |       298 | {"strand": null, "featureloc_id": 3260896, "variant_type_id": 796} |          396318
       327991 | LcC09269p298 | SNP          |       3400407 | LcChr1          |    501710 |    501711 | {"strand": -1, "featureloc_id": 4562011, "variant_type_id": 796}   |          396319
       327991 | LcC09269p298 | SNP          |       3400411 | LcChr5          | 250136623 | 250136624 | {"strand": -1, "featureloc_id": 4562010, "variant_type_id": 796}   |          396320
       327991 | LcC09269p298 | SNP          |       3400411 | LcChr5          | 250519947 | 250519948 | {"strand": -1, "featureloc_id": 4562009, "variant_type_id": 796}   |          396321
  (4 rows)
