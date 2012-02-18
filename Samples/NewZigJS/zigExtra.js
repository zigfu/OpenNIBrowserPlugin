//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
function include(file)
{
	var script  = document.createElement('script');
	script.src  = file;
	script.type = 'text/javascript';
	script.defer = true;
	document.getElementsByTagName('head').item(0).appendChild(script);
}

include("sylvester.js");

function sumMatrix(mat) {
	var sum = 0;
	elements = mat.elements
	for(var i in elements) {
		for(var j in elements[i]) {
			sum += elements[i][j];
		}
	}
	return sum;
}
// Reference: Oliver K. Smith: Eigenvalues of a symmetric 3 × 3 matrix. Commun. ACM 4(4): 168 (1961) 
// find the eigenvalues of a 3x3 symmetric matrix
function getEigenvalues(mat) {
	var m = mat.trace() / 3;
    var K = mat.subtract( Matrix.I(3).x(m)); // K = mat - I*tr(mat)
    var q = K.determinant() / 2;
    var tempForm = K.x(K);
 
	var p = sumMatrix(tempForm) / 6;
 
    // NB in Smith's paper he uses phi = (1./3.)*arctan(sqrt(p**3 - q**2)/q), which is equivalent to below:
    var phi = (1/3)*Math.acos(q/Math.sqrt(p*p*p));
 
    if (Math.abs(q) >= Math.abs(Math.sqrt(p*p*p))) {
        phi = 0;
	}
 
    if (phi < 0) {
        phi = phi + Math.PI/3;
	}
 
    var eig1 = m + 2*Math.sqrt(p)*Math.cos(phi);
    var eig2 = m - Math.sqrt(p)*(Math.cos(phi) + Math.sqrt(3)*Math.sin(phi));
    var eig3 = m - Math.sqrt(p)*(Math.cos(phi) - Math.sqrt(3)*Math.sin(phi));
 
    return [eig1, eig2, eig3];
}

function getCofactorMatrix(mat) {
	var dims = mat.dimensions();
	var xSize = dims.cols;
	var ySize = dims.rows;
	var output = mat.map(function(x, i, j) { return mat.minor(i+1,j+1,xSize-1, ySize-1).determinant(); } );
	return output;
}

function getStddevs(vectors) {
	if (vectors.length == 0) { return []; }
	var sum = Vector.Zero(vectors[0].dimensions());
	for(k in vectors) {
		sum = sum.add(vectors[k]);
	}
	var avg = sum.multiply(1/(vectors.length));
	var covarianceMatrix = Matrix.Zero(avg.dimensions(), avg.dimensions());
	for(k in vectors) {
		var temp = vectors[k].subtract(avg);
		covarianceMatrix = covarianceMatrix.map(function(x, i, j) { return x + temp.elements[i-1]*temp.elements[j-1]; } );
	}
	var values = getEigenvalues(covarianceMatrix);
	for (key in values) {
		values[key] = Math.sqrt(Math.abs(values[key]));
	}
	return values;
}

/*
//TODO: finish this code conversion
function getLargestColumn(mat) {

}

function getEigenvectors(mat, eigenvalues) {
	var lambda = eigenvalues;
    T tiny = tolerance * ::MaxAbs(lambda.x,lambda.z);
    if(lambda.x-lambda.y<=tiny){
        if(lambda.y-lambda.z<=tiny)
			eigenvectors = Matrix3X3<T>::Identity();
        else {
            Vector3D<T> v2=getCofactorMatrix(mat.map(function(x) { return x-lambda.z; })).LargestColumnNormalized();
            Vector3D<T> v1=v2.UnitOrthogonalVector();
            eigenvectors = Matrix3X3<T>(CrossProduct(v1,v2),v1,v2);
		}
	}
    else if(lambda.y-lambda.z<=tiny) {
        Vector3D<T> v0=((*this)-lambda.x).CofactorMatrix().LargestColumnNormalized();
        Vector3D<T> v1=v0.UnitOrthogonalVector();
        eigenvectors = Matrix3X3<T>(v0,v1,CrossProduct(v0,v1));
	}
    else {
        Vector3D<T> v0=((*this)-lambda.x).CofactorMatrix().LargestColumnNormalized();
        Vector3D<T> v2=((*this)-lambda.z).CofactorMatrix().LargestColumnNormalized();
        eigenvectors = Matrix3X3<T>(v0,CrossProduct(v2,v0),v2);
	}

}
*/