# Natural Diversity Genotypes
This module provides support and visualization of genotypic data stored in a modified GMOD Chado schema. The 3.x branch of this module represents a shift towards support for **large scale genotypic datasets** through backwards compatible improvements to the Chado schema including a new gathering table for genotypes (genotype_call) modelled after the chado phenotype table, optimized queries and well-choosen indexes. For benchmarking demonstrating the efficiency of this module, see our [wiki](https://github.com/UofS-Pulse-Binfo/nd_genotypes/wiki/Benchmarking).

**Tripal 3 compatible release projected for August 2017**

## Dependencies
1. Tripal 3.x
2. Tripal Download API
3. PostgreSQL 9.3 (9.4 recommended)

## Installation
1. Install the dependencies listed above.
2. Install this module as you would any other Drupal module.

## Usage
1. Configure this module by going to Administration » Tripal » Extensions » Natural Diversity Genotypes. Specifically, you need to set which content types map to variants and markers.
2. Load data using the [genotype loader](https://github.com/UofS-Pulse-Binfo/genotypes_loader). Since the Genotype loader is not yet released, we highly suggest test loading each dataset on a development site.
3. You can access the genotype matrix at [your drupal site]/chado/genotypes/[genus].
4. You should see a "Genotypes" and updated "Sequences" pane on the Marker and Variant pages configured in #1.

## Functionality
- Extensive configuration allowing for flexiblity in ontology terms used, as well as, colours and wording used in visualizations.
- Multiple Tripal 3 Fields which provide flexible, configurable summaries of genotypic data.
    - Marker/Variant Genotype Summary: a pie chart showing the ratio of alleles recorded per marker.
    - Marker/Variant Flanking Sequence: a FASTA record showing flanking sequence with all known variants indicated via IUPAC codes (useful in marker design).
    - Marker List: provides links to the markers assaying a given variant.
    - Project Genotype Summary (comming soon): provides a summary of all genotypic data associated with a given project.
    - Germplasm Genotype Summary (comming soon): provides a summary of all genotypic data collected from a given germplasm.
    - Genotype Matrix Quick Search (comming soon): provides a quick search for marker, variant, project and germplasm pages which redirects to a filtered genotype matrix.
- Genotype Matrix search allowing users to extract genotypes for a user-defined set of germplasm. Includes filtering by marker/variant type, variant location, and pairwise polymorphism. Filtering by quality is comming soon.
- Integration of all fields with Tripal 3 web services allowing you to share your genotypic data with other groups.

### Genotype Matrix
This module provides genotype search functionality that allows users to select which germplasm and variants they are interested in and be shown a colour-coded variant by germplasm table which can be further filtered by marker/variant type and to only show polymorphic variants (pairwise comparison choosen by the user). After filtering to their desired dataset, the user can download the table as a tab-delimited file.

The following screenshot shows the form the user is presented with for choosing filter criteria:
![genotypematrix_form](https://cloud.githubusercontent.com/assets/1566301/19090330/1d656d6c-8a3b-11e6-8776-6f3c1e10e18b.png)

The following screenshot shows the table of genotype calls presented to the user based on their filter criteria:
![genotypematrix_table](https://cloud.githubusercontent.com/assets/1566301/19090346/2e83e0b0-8a3b-11e6-9ad3-9574aa88b7e5.png)

## Data Storage
Genotypic data is stored through use of a custom table (genotype_call) created by this module. This table provides a centralized, relational table which pulls all the information for a given genotypic call (marker assay result on a given germplasm for a specific project) together in a single record. It also supports flexible storage for all meta-data associated with a genotype assay result through a PostgreSQL JSONB metadata column.
![](http://creately.com/jupiter/diagram/image/ipkbi8wt)

For a comparison of this genotypic data storage method with the Chado Natural Diversity module, see our [wiki](https://github.com/UofS-Pulse-Binfo/nd_genotypes/wiki/How-to-Store-your-Data).
