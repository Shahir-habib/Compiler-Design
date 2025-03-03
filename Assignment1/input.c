
int main(){
    //Code for finding factorial of a number
    int n;
    int fact = 1;
    printf("Enter a number: ");
    scanf("%d", &n);
    for(int i = 1; i <= n; i++){
        fact *= i;
    }
    printf("Factorial of %d is %d\n", n, fact);
    return 0;
}
