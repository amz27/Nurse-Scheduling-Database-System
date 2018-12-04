# Nurse-Scheduling-Database-System
  Nurse scheduling problem is the operations research problem of finding an optimal way to assign nurses to shifts with a set of constraints. Each nurse has their own restrictions, as does the hospital. The problem is described as finding a schedule that both respects the constraints of the nurses and fulfills the objectives of the hospital. In this particular cases that we are working on, we focus on the rules that a nurse can work 3 shifts: day shift, night shift and late night shift.
  The system contains various functions for employee aspects, department aspects and manager aspects. The implementation of the system has been optimized to the best by adding index and using binary tree search. The system also has output generator so user will face data that is rearrange in human-readable format. The outcome should be clear and easy to read for people from any fields.
  
Specification
1.	Employee: store basic information for each individual employee. Such as last name, first name, phone number. It also include departID to specify which department he or she belongs to. EmployeeID is the primary key in the Employee schema.
2.	Salary: In salary schema, salaryID is the primary key. We have the information of salary per shift and roleID to find the specific role that the employee has. The information is used to calculator labor fee for certain time interval or other possible purpose.
3.	Role: In the role schema, it contains roleID and role name. Each roleID will match with one role name. It is used to specify salary and schedule employees equally.
4.	Schedule: Schedule schema contains schedule ID, role ID, depart ID, shift ID and salary ID. It has the connect with all the other schemas in order to get the outcome.
5.	Department: The table contains department name, department ID and the number of beds per department. People can find the information of department by choosing the department in the select statements. 
6.	Shift: The table use shift ID as a primary key. It contains shift date and the starting and ending time for its shift date. It will also have the number of the day that people have worked. So manager can get the worker’s salary by counting this.

Sample Data:
1.	Employee (employeeID |Lname |Fname |Mname |address |city |state |zip |FTorPT |salary |phone |roleID |certification |departID )
    Ex:97002,Richard,mars,n/a,n/a,University Park,PA,16803,ft,20000,222222222,200,n/a,1002
2.	Shift (shiftID |shiftDate |startTime |endTime |day)
    Ex:
3.	Schedule (departID |shiftID |employeeID |roleID |salaryID ): 3.	Schedule (departID |shiftID |employeeID |roleID |salaryID ): 

4.	Role (roleID| roleName ):

5.	Department (departID| departName |bed): 
  
6.	Salary (salaryID |salaryPerShift | roleID):


The design is an approach to solve a mini Nurse Scheduling Problem (NSP). It includes data for one week for each employee’s schedule including date, days, shift, department and role; for each department’s needs including needs per role and number needed; for each department’s scheduled people with their basic information. Besides, because all the data are stored in system, there are more conditions we can have when necessary. The database management is done in MySQL, using the database to organize all the information and get the data we need. Then, using index while searching for a specific information will help to improve performance of the running time. Not only fast computer, but also small laptop will be able to handle big amount of data. 
