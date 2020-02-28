
Benchmarking
============

We decided to do more formal benchmarking on two of our modules for the ISMB 2017 Conference. The details of such are included here for the benefit of the community :-).

Caveats
-------

 - All timings were done on the same hardware (see specification below).
 - Queries were timed at the database level using PostgreSQL 9.4.10 EXPLAIN ANALYZE [query] and as such don't include rendering time in Tripal. Note: the addition of the analyze keyword ensures the query is actually run and the actual total time was reported.
 - The system the tests were run on includes a production Tripal site with small and uneven load. The tests were run 3 times on the same day over the span of at least 4 hours to help mitigate the differences in load.
 - Datasets are computationally derived with no missing data points.

Timings
-------

Timings were done on July 15, 2017:

======== ==================== ============= ============= ============= =============
Dataset  Query                Rep1           Rep2          Rep3          Average
======== ==================== ============= ============= ============= =============
#1        Unfiltered Query     3.988 ms     4.484 ms      4.456 ms      4.309 ms
#1        Range Query          4.788 ms     4.771 ms      4.77 ms       4.776 ms
#1        Polymorphic Filter   12.511 ms    13.548 ms     13.428 ms     13.16 ms
#2        Unfiltered Query     6009.39 ms   6741.178 ms   6034.41 ms    6261.659 ms
#2        Range Query          6538.921 ms  6173.15 ms    6203.673 ms   6305.248 ms
#2        Polymorphic Filter   751.286 ms   601.563 ms    607.435 ms    653.428 ms
#3        Unfiltered Query     5.082 ms     6.671 ms      3.424 ms      5.059
#3        Range Query          6.870 ms     7.582 ms      3.660 ms      6.037
#3        Polymorphic Filter   68.305 ms    74.741 ms     65.071 ms     69.372
======== ==================== ============= ============= ============= =============

NOTE: Each replicate shows both queries executed to extract the data for the genotype matrix.

 - See "Datasets" for a description of the datasets the tests were run on and how they were generated.
 - See "Queries" section below for the explanation on why two queries are used and for the exact queries executed.
 - See "Hardware" section for the specification of the database server all tests were run on.
 - Script used to determine timings: https://gist.github.com/laceysanderson/d7b0090a22764e807844d9e74276f02d

Datasets
--------

The queries were tested on two SNP datasets with different composition.

Datasets #1 and #2 were generated using the Generate Tripal Data Drush module; specifically, the drush generate-genotypes command. While the data is computationally derived, it does attempt to simulate real data by choosing allele frequencies (10-60% AA; 0-20% AB) per SNP before generating calls. However, in contrast to real datasets, the test datasets do not include missing calls.

Dataset #3 is a real world dataset from our research group. Details on this dataset can be found below as well as in `E Ogutcen, et al. (2018) Applications in Plant Sciences 6(7):e01165 <https://doi.org/10.1002/aps3.1165>`_.

============ ============= ======= ===============
Name 	     SNPs          Samples Genotype Calls
============ ============= ======= ===============
Dataset #1   45 million    45      2.03 billion
Dataset #2   445 thousand  4500    2.03 billion
Dataset #3   372,506       534     105,340,269
============ ============= ======= ===============

NOTE: Dataset #1 took 12591m43.393s to load for a final database size of 1135 GB not including published pages.

Queries
-------

The queries tested represent those executed by the genotype matrix provided by this module. An important thing to keep in mind is that the genotype matrix results are actually retrieved by two queries.

1. Retrieves 100 variants that match the specified filter criteria. This changes between tests based on filter criteria (see expanded query under each test).
2. Retrieves the genotype calls for the specified variant by germplasm combination for the current page. This query remains static across tests.

.. code-block:: sql

  SELECT variant_id, germplasm_id, allele_call
  FROM unnest(ARRAY[
      -- comma-separated list of variant_ids retrieved by the first query
    ]) variant_id
  JOIN chado.mview_ndg_lens_calls call USING(variant_id)
  WHERE germplasm_id IN (:germplasm_0, :germplasm_1, :germplasm_2, :germplasm_3, :germplasm_4, :germplasm_5, :germplasm_6, :germplasm_7, :germplasm_8, :germplasm_9)


The timings for both queries were determined for each replicate and were added together before averaging of the replicates. This was done to provide the total query time to retrieve data for the genotype matrix per criteria tested.
Unfiltered Query

Unfiltered Query
^^^^^^^^^^^^^^^^^

This is the query executed when you specify a set of germplasm to see genotypes for but do not specify any other criteria. Specifically, the following query represents the case where 10 germplasm have been specified (denoted by :germplasm_0 through :germplasm_9 placeholders in the query).

.. code-block:: sql

  SELECT v.*, cf.nid
  FROM chado.mview_ndg_lens_variants v
  LEFT JOIN public.chado_feature cf ON cf.feature_id=v.variant_id
  WHERE
    EXISTS (
      SELECT true FROM chado.mview_ndg_lens_calls call
      WHERE germplasm_id IN (:germplasm_0, :germplasm_1, :germplasm_2, :germplasm_3, :germplasm_4, :germplasm_5, :germplasm_6, :germplasm_7, :germplasm_8, :germplasm_9)
        AND call.variant_id=v.variant_id )
  ORDER BY srcfeature_name ASC, fmin ASC
  LIMIT 100

Range Query
^^^^^^^^^^^

This is the query executed when you restrict the set of variants returned to a specific region in the genome. Specifically, the range requested is specified by start chromosome(:sbackbone) and position(:sfmin) to end chromosome(:ebackbone) and position(:efmin) in the query below. Additionally the query includes the same 10 germplasm used for the "Unfiltered Query" (denoted by :germplasm_0 through :germplasm_9 placeholders in the query).

.. code-block:: sql

  SELECT v.*, cf.nid
  FROM chado.mview_ndg_lens_variants v
  LEFT JOIN public.chado_feature cf ON cf.feature_id=v.variant_id
  WHERE
    ROW(v.srcfeature_name, v.fmin) BETWEEN ROW(:sbackbone, :sfmin) AND ROW(:ebackbone,:efmin)
    AND EXISTS (
      SELECT true FROM chado.mview_ndg_lens_calls call
      WHERE germplasm_id IN (:germplasm_0, :germplasm_1, :germplasm_2, :germplasm_3, :germplasm_4, :germplasm_5, :germplasm_6, :germplasm_7, :germplasm_8, :germplasm_9)
        AND call.variant_id=v.variant_id )
  ORDER BY srcfeature_name ASC, fmin ASC
  LIMIT 100

Polymorphic Filter
^^^^^^^^^^^^^^^^^^

This is the query executed when you indicate that you would like only variants that are polymorphic returned. Specifically, only variants with different genotype calls between two given germplasm (:poly1 and :poly2) will be shown. Additionally the query includes the same 10 germplasm used for the "Unfiltered Query" (denoted by :germplasm_0 through :germplasm_9 placeholders in the query). Notice that it is much faster to specify that the variant should not be monomorphic then that it should be polymorphic.

.. code-block:: sql

  SELECT v.*, cf.nid
  FROM chado.mview_ndg_lens_variants v
  LEFT JOIN public.chado_feature cf ON cf.feature_id=v.variant_id
  LEFT JOIN (
      SELECT a.variant_id, a.allele_call=b.allele_call as monomorphic
      FROM mview_ndg_lens_calls a, mview_ndg_lens_calls b
      WHERE
        a.variant_id=b.variant_id
        AND a.germplasm_id=:poly1
        AND b.germplasm_id=:poly2
    ) p ON p.variant_id=v.variant_id
  WHERE
    p.monomorphic IS FALSE
    AND EXISTS (
      SELECT true FROM chado.mview_ndg_lens_calls call
      WHERE germplasm_id IN (:germplasm_0, :germplasm_1, :germplasm_2, :germplasm_3, :germplasm_4, :germplasm_5, :germplasm_6, :germplasm_7, :germplasm_8, :germplasm_9)
        AND call.variant_id=v.variant_id )
  ORDER BY srcfeature_name ASC, fmin ASC
  LIMIT 100

System Specification
--------------------

Our Production Tripal site is setup on a dedicated two-box system (webserver + database server) with Apache + PHP installed on the first box and PostgreSQL installed on the second box. All testing for this benchmarking was done on a clean Tripal v3 site setup on the same two boxes in order to show queries time on a Production Server versus a less powerful Development server.

 - RAID 10 configuration
 - Debian GNU/Linux 8.7 (jessie)
 - PostgreSQL 9.4.10
 - Minimal PostgreSQL configuration tuning
 - Hardware Specification (Database Server only)
    - Lenovo X3650 M5 2U Rackmount
    - Server 2x Xeon 6C E52643 V3 3.4GHz
    - 128GB RAM (8x 16GB TruDDR4 Memory (2Rx4, 1.2V) LP RDIMM) 1x ServeRAID M5210 Controller w/ 1GB Flash/RAID 5 Upgrade
    - 8x 600GB 15K 6Gbps SAS 2.5in G3HS HDD
    - Redundant Power Supplies
    - 4x 1GbE Onboard Ethernet
