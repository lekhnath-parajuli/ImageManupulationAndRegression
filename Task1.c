#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
struct DataSet {
	int x;
	int y;
};


////////////////////////////////////////////////////////////////////////////////
struct variables {
	int x;
	int x_square;
	int y;
	int xy;
};


////////////////////////////////////////////////////////////////////////////////
struct all_sums {
	int X_sum;
	int Y_sum;
	int XY_sum;
	int X_square_sum;
};

////////////////////////////////////////////////////////////////////////////////
struct linear_regression {
	float m;
	float b;
};

int TotalDatasetLength;

////////////////////////////////////////////////////////////////////////////////
void combin_files() {
	FILE *file1_len = fopen("./LinearRegressionDataset/datasetLR1.txt", "r");
	FILE *file2_len = fopen("./LinearRegressionDataset/datasetLR2.txt", "r");
	FILE *file3_len = fopen("./LinearRegressionDataset/datasetLR3.txt", "r");
	FILE *file4_len = fopen("./LinearRegressionDataset/datasetLR4.txt", "r");
	remove("./LinearRegressionDataset/Task1.txt");
	FILE *final_dataset = fopen("./LinearRegressionDataset/Task1.txt", "a");

	struct DataSet coordinates;
	
		while(!feof(file1_len)) {											   
			fscanf(file1_len, "%d,%d\n", &coordinates.x, &coordinates.y);
			TotalDatasetLength += 1;
			fprintf(final_dataset, "%d,%d\n", coordinates.x, coordinates.y);
		} fclose(file1_len);																	
		
		while(!feof(file2_len)) {
			fscanf(file2_len, "%d,%d\n", &coordinates.x, &coordinates.y);
			TotalDatasetLength += 1;
			fprintf(final_dataset, "%d,%d\n", coordinates.x, coordinates.y);
		} fclose(file2_len);
		
		while(!feof(file3_len)) {
			fscanf(file3_len, "%d,%d\n", &coordinates.x, &coordinates.y);
			TotalDatasetLength += 1;
			fprintf(final_dataset, "%d,%d\n", coordinates.x, coordinates.y);
		} fclose(file3_len);
		
		while(!feof(file4_len)) {
			fscanf(file4_len, "%d,%d\n", &coordinates.x, &coordinates.y);
			TotalDatasetLength += 1;
			fprintf(final_dataset, "%d,%d\n", coordinates.x, coordinates.y);		
		} fclose(file4_len);
		
	fclose(final_dataset);
}


////////////////////////////////////////////////////////////////////////////////
void* Load_Data() {
	int index = -1;
	struct DataSet coordinates;
	struct DataSet *datas = (struct DataSet *) malloc(sizeof(struct DataSet) * TotalDatasetLength);
	
		// loading all datas and puting it in array of structure                  
		FILE *Task1 = fopen("./LinearRegressionDataset/Task1.txt", "r+");
		
		while(!feof(Task1)) {										   
			fscanf(Task1, "%d,%d\n", &coordinates.x, &coordinates.y);
			index += 1;	
			datas[index] = coordinates;
		} fclose(Task1);							   

	return datas;
}


////////////////////////////////////////////////////////////////////////////////
void* Process_all_values_for_variables(struct DataSet *x_and_y) {
	int i = 0;
	struct variables *var = (struct variables *) malloc(sizeof(struct variables)* TotalDatasetLength);
	struct variables temp;
		
		for(i=0; i<TotalDatasetLength; i++) {
			int x, y;
			
			x               =   x_and_y[i].x;
			y               =   x_and_y[i].y;
			temp.x          =   x;
			temp.y          =   y;
			temp.x_square   =   (x * x);
			temp.xy         =   (x * y);

			var[i] = temp;
		}
	return var;
}


////////////////////////////////////////////////////////////////////////////////
struct all_sums find_all_sums(struct variables *var) {
	struct all_sums ref = { 0, 0, 0, 0};
//	struct all_sums *sums = &ref;
	int i = 0;
		for(i=0; i<TotalDatasetLength; i++) {
			
			 ref.X_sum        	+= var[i].x;
			 ref.Y_sum        	+= var[i].y;	
			 ref.XY_sum       	+=  (var[i].x * var[i].y);
			 ref.X_square_sum 	+=  var[i].x * var[i].x;
			 
		}
		
	return ref;
}

////////////////////////////////////////////////////////////////////////////////
void* calculate_m_and_b(struct all_sums sums) {
	struct linear_regression *m_and_b = (struct linear_regression*) malloc(sizeof(struct linear_regression));
	float num;
	float denum;
	float y  = sums.Y_sum;
	float x  = sums.X_sum;
	float xx = sums.X_square_sum;
	float xy = sums.XY_sum;
	float n  = TotalDatasetLength;
	
		num 		= 	(n*xy) - (x * y);
		denum 		= 	(n*xx) - (x*x);
		m_and_b->m 	= 	num/denum;
		
		num 		= 	(y*xx) - (x * xy);
		denum 		= 	(n*xx) - (x*x);
		m_and_b->b 	= 	num/denum;	
	
	return m_and_b;
}

////////////////////////////////////////////////////////////////////////////////
void predict(struct linear_regression *m_and_b) {
	float X;
	float Y;
	float m = m_and_b->m;
	float b = m_and_b->b;
	
		printf("-----------------------------------------------\n");
		printf("| Predict the value of Y using the Value of X |\n");
		printf("-----------------------------------------------\n");
		
		printf("\nEnter a value for X ::> ");
		scanf("%f", &X);
		
		Y = m*X + b;
		printf("Predicted Value is:: %f", Y);
}

////////////////////////////////////////////////////////////////////////////////
void main() {
	combin_files();

	struct DataSet *x_and_y  		 	=  (struct DataSet *)    			Load_Data();
	struct variables *var    		 	=  (struct variables *) 			Process_all_values_for_variables(x_and_y);
	struct all_sums sums   		 		=  find_all_sums(var);
	struct linear_regression *m_and_b   =  (struct linear_regression *) 	calculate_m_and_b(sums);
	
	predict(m_and_b);
}


