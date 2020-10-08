InDesign plugin that count lines of text in active document.


HOW TO RUN.
To run this plugin you must have on your computer:
1) Adobe InDesign program of version cs6 and newer.
2) SDK for that Indesign version.

You must specify path to Indesign SDK
and you should specify path to Adobe Indesign program in project properties.

Output of this plugin saves in $(YourInDesignFolder)\Plugins\LinesCounter\ as .pln file.
/* if you don't specify path to InDesign, you must move .pln file to that folder manually */


HOW TO USE.
1)  Open your InDesign program.
2)  Open document for counting text lines.
3)  In top menu click Plug-Ins -> Intexsoft -> Lines Counter -> Run.
4)  Dialog menu would be opened.
    There you see input-box. 
    4.1)  Leave it blank, if you like to count text lines on every page in document.
    4.2)  Write page numbers in input-box to specify pages for counting text lines.
    
          It supports pages enumerations separated by comma "," and pages intervals dash separated "-".
          Space symbols are not taken into account.
          /* Format like you specify pages for printing in Microsoft Word, example: 1, 3-5, 8-12, 15 */
          
          You may encounter exceptions when:
            specifying pages out of document range,
            specifying invalid intervals,
            have no active document in InDesign proram
          
5) Click OkButton. Output result would be in dialog.

