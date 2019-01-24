
Chado Schema and Extensions
===========================

All of the tools provided by this module retrieve their data from two question-agnostic materialized views. This provides a significant performance boost, as well as supports flexibility in the ways you can store your data.

There are currently two ways to store your genotypic data in Chado v1.3 with this module providing a third, more efficient way. While this module only supports Method #2, it can easily support data stored using the other two methods via custom queries that populate the materialized views with your data. You can see a comparison of the various methods below which should make it clear why we've gone with the storage method we have. Furthermore, you can see benchmarking for Method #2 here: https://github.com/UofS-Pulse-Binfo/nd_genotypes/wiki/Benchmarking.

Comparison of Methods
---------------------

+--------+----------------+---------------+--------------------+----------+---------------------------------+
| Method | Name           | Custom Tables | Supports Meta-data | # Tables | Comments                        |
+========+================+===============+====================+==========+=================================+
| 1      | ND Experiment  | No            | Yes                | 14       | Not suitable beyond 3 million   | 
|        |                |               |                    |          | genotype calls.                 |
+--------+----------------+---------------+--------------------+----------+---------------------------------+ 
| 2      | Genotype Call  | Yes           | Yes                | 10       | Most efficient; although it     |
|        |                |               |                    |          | touches the same number of      |
|        |                |               |                    |          | tables as Method #3 there are   | 
|        |                |               |                    |          | less records per genotype call  |
+--------+----------------+---------------+--------------------+----------+---------------------------------+
| 3      | Stock Genotype | No            | No                 | 10       | A good alternative if you don't |
|        |                |               |                    |          | want to use custom tables but   |
|        |                |               |                    |          | have a lot of data. Similar     |
|        |                |               |                    |          | efficiency to Method #2 but     |
|        |                |               |                    |          | less support for meta-data.     |
+--------+----------------+---------------+--------------------+----------+---------------------------------+

**All three methods store Markers & Variants in the same way.** For the purposes of this module, a variant is a location on the genome where variation has been detected and has a type of SNP, MNP, Indel, etc. A marker then indicates which method the genotype calls associated with it were determined by. For example, you may have a variant on Chromosome 1 at position 45678 that you detected variation through two different methods. Each method would be indicated as a marker and all the genotype calls detected by that method would be attached to the appropriate marker and not directly to the variant. This has been determined necessary since the level of trust and how you interpret any quality meta-data will depend on the method.

Method 1: The Chado Natural Diversity Experiment Tables.
-----------------------------------------------------------

This is the first method that was supported and the only method supported the for the 1.x versions of this module.

.. image:: http://creately.com/jupiter/diagram/image/ipk5kqpu2

To try to give you an idea of the records needed we will consider a single line in a VCF file where there are only three alleles and six stocks:

+-----------+----------------------------+-----------------------------+---------------------------------------+
| # Records | Tables                     | Example                     | Explanation                           |
+===========+============================+=============================+=======================================+
| 2         | feature                    | "LcChr1p555" and            | One each for variant and marker       |
|           |                            | "LcChr1p555 GBS Marker"     | where the variant may already exist.  |
+-----------+----------------------------+-----------------------------+---------------------------------------+
| 2         | featureloc                 | Chr1:554-555 for each.      | Locate each of the variant and        |
|           |                            |                             | marker on the chromsome.              |
+-----------+----------------------------+-----------------------------+---------------------------------------+
| 1         | feature_relationship       | "LcChr1p555 GBS Marker"     | Link the marker and variant.          |
|           |                            | is_marker_of "LcChr1p555"   |                                       |
+-----------+----------------------------+-----------------------------+---------------------------------------+ 
| 6         | genotype, feature_genotype | "AA", "AC", "CC"            | One genotype record per unique        |
|           |                            |                             | allele call. NOTE: the allele call    |
|           |                            |                             | must be unique to the marker in       |
|           |                            |                             | order to be able to trace from        |
|           |                            |                             | marker to stock. Thus there will be   |
|           |                            |                             | a record for "AA" for marker5 and a   |
|           |                            |                             | separate record for "AA" for          |
|           |                            |                             | marker9.                              |
+-----------+----------------------------+-----------------------------+---------------------------------------+
| 18        | nd_experiment_genotype,    | All Foreign Keys            | Three records per stock in order to   | 
|           | nd_experiment,             |                             | link the stock to it's allele through |
|           | nd_experiment_stock        |                             | through the natural diversity tables. |
+-----------+----------------------------+-----------------------------+---------------------------------------+
| 6         | nd_experiment_project      | Again Foreign Keys          | One per nd_experiment to link it to   |
|           |                            |                             | the project. Note there will be one   |
|           |                            |                             | nd_experiment per stock/marker        |
|           |                            |                             | combination.                          |
+-----------+----------------------------+-----------------------------+---------------------------------------+

**Total: 35 records per line in a VCF with only 6 stocks and 3 alleles per variant.**

Thus if your VCF file has 100,000 lines you will have to create 3,500,000 records across 12 tables to store it. Keep in mind that number doesn't include the records for your chromosomes or for your stocks since the first likely already exists and the second is only entered once per file.

Method 2: Custom Genotype Call Table.
-------------------------------------

.. image:: http://creately.com/jupiter/diagram/image/ipkbi8wt

Now, lets consider the same example as in Method 1 (one VCF line with three alleles and six samples):

+-----------+----------------------+-------------------------------+-------------------------------------------+
| # Records | Tables               | Example                       | Explanation                               |
+===========+======================+===============================+===========================================+
| 2         | feature              | "LcChr1p555" and              | One each for variant and marker where the |
|           |                      | "LcChr1p555 GBS Marker"       | variant may already exist.                |
+-----------+----------------------+-------------------------------+-------------------------------------------+
| 2         | featureloc           | Chr1:554-555 for each.        | Locate each of the variant and marker on  |
|           |                      |                               | the chromsome.                            |
+-----------+----------------------+-------------------------------+-------------------------------------------+
| 1         | feature_relationship | "LcChr1p555 GBS Marker"       | Link the marker and variant.              |
|           |                      | is_marker_of "LcChr1p555"     |                                           |
+-----------+----------------------+-------------------------------+-------------------------------------------+
| 6         | genotype_call        | All Foreign Keys with the     | This links the marker, variant, allele    |
|           |                      | exception of any quality      | call, stock and project all in one and    |
|           |                      | information you want to store | stores any addition quality information   |
|           |                      | in the meta-data column       | in the meta-data column.                  |
+-----------+----------------------+-------------------------------+-------------------------------------------+

**Total: 11 records per line in a VCF with only 6 stocks and 3 alleles per variant.**

Notice how much more efficient this method is. This is because (1) most of the foreign key connections are taking place in a single table (genotype_call) and (2) there now only needs to be a single record in the genotype table for "AA" rather than one record per marker using the previous method. For further comparison, the same 100,000 line VCF file would now only take 1,100,000 records to store not including the records for your chromosomes, which already exist, those for your stocks, only 6 per file, and those for you alleles (genotype table), which likely already exist. Furthermore, storing meta-data doesn't increase the number of records like it would in the first method.

Method 3: via Stock Genotype Table.
-----------------------------------

.. image:: http://creately.com/jupiter/diagram/image/ipkh4bqy

Finally, lets consider the last method using the same example (one VCF line with three alleles and six samples):

+-----------+----------------------------+-------------------------------+-------------------------------------------+
| # Records | Tables                     | Example                       | Explanation                               |
+===========+============================+===============================+===========================================+
| 2         | feature                    | "LcChr1p555" and              | One each for variant and marker where     |
|           |                            | "LcChr1p555 GBS Marker"       | the variant may already exist.            |
+-----------+----------------------------+-------------------------------+-------------------------------------------+                                                                                                                                                                      
| 2         | featureloc                 | Chr1:554-555 for each.        | Locate each of the variant and marker on  |
|           |                            |                               | the chromsome.                            | 
| 1         | feature_relationship       | "LcChr1p555 GBS Marker"       | Link the marker and variant.              |
|           |                            | is_marker_of "LcChr1p555"     |                                           |                                                                                                                                                                                                               
+-----------+----------------------------+-------------------------------+-------------------------------------------+
| 6         | genotype, feature_genotype | "AA", "AC", "CC"              | One genotype record per unique allele     |
|           |                            |                               | call. NOTE: the allele call must be       |
|           |                            |                               | unique to the marker in order to be able  |
|           |                            |                               | to trace from marker to stock. Thus there |
|           |                            |                               | will be a record for "AA" for marker5 and |
|           |                            |                               | a separate record for "AA" for marker9.   |
+-----------+----------------------------+-------------------------------+-------------------------------------------+
| 6         | stock_genotype             | All Foreign Keys              | Link each DNA stock to the allele detected|
|           |                            |                               | using the assay. We are only counting the |
|           |                            |                               | linking records here since the stocks will|
|           |                            |                               | only be created once per file.            |  
+-----------+----------------------------+-------------------------------+-------------------------------------------+

**Total: 17 records per line in a VCF with only 6 stocks and 3 alleles per variant.**

This is a good mid-range option that allows you to store genotypes efficiently without the use of any custom tables! The trade-off is that there isn't a good way to store meta-data related to the assay such as read depth. To complete the comparison, the same 100,000 line VCF file would take 1,700,000 records to store not including the records for your chromosomes, which already exist, those for your stocks, only 6 per file.
