# Socket-Spreadsheet

The program should allow the users to enter numeric values, text, or formulas into any cell at any time. As new data is entered, the spreadsheet should be updated and re-displayed to all clients/users. The program runs until the first user quits. The server keeps track of a count of the number of clients and this information is communicated to all the clients/users upon each re-display.

On the 9 x 9 cells spreadsheet, rows are labeled with numbers, starting from 1, and columns are labeled with letters, starting from A. A Cell is referenced
by a letter number pair, with no space or symbol between; for example, E9. 

The program mimics basic functionalities of an excel spreadsheet. The client isn't prompt on what type of entry is being made into a cell. It prompt only for an input. If the user types data consisting only of numeric values, for example 4.53, the program recognize that input as a number. 

If the user types input describing a formula, the program recognize that input as a formula. Everything else defaults to text.

If text entries or other displayed content go beyond the given display size, the display should be truncated.

Formulas are of the form =AVERAGE(A2,A5). The accepted formulas are SUM, RANGE, and AVERAGE.
