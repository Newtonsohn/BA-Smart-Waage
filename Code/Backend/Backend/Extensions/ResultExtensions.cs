using Backend.Domain.Kernel;

namespace Backend.API.Extensions
{
    public static class ResultExtensions
    {
        public static IResult MapToHttpResult(this Result result)
        {
            return result.IsSuccess ? Results.Ok() : result.ToProblemDetails();
        }

        public static IResult MapToHttpResult<T>(this Result<T> result)
        {
            return result.IsSuccess ? Results.Ok(result.Value) : result.ToProblemDetails();
        }

        public static IResult ToProblemDetails(this Result result)
        {
            if (result.IsSuccess)
                throw new InvalidOperationException();

            return result.Error.Type switch
            {
                ErrorType.Validation => Results.ValidationProblem(new Dictionary<string, string[]>
                {
                    ["General"] = new[] { result.Error.Description }
                }, statusCode: GetStatusCode(result.Error.Type)),

                _ => Results.Problem(
                    statusCode: GetStatusCode(result.Error.Type),
                    title: GetTitle(result.Error.Type),
                    detail: result.Error.Type.ToString(),
                    type: "https://tools.ietf.org/html/rfc7231#section-6.5",
                    extensions:
                        [
                            new KeyValuePair<string, object?>("errors",  result.Error)
                        ]),
            };

        }

        private static int GetStatusCode(ErrorType errorType)
        {
            return errorType switch
            {
                ErrorType.Validation => StatusCodes.Status400BadRequest,
                ErrorType.NotFound => StatusCodes.Status404NotFound,
                ErrorType.Conflict => StatusCodes.Status409Conflict,
                _ => StatusCodes.Status500InternalServerError
            };
        }

        private static string GetTitle(ErrorType errorType)
        {
            // Example switch (adapt as needed)
            return errorType switch
            {
                ErrorType.Validation => "Bad Request",
                ErrorType.NotFound => "Resource Not Found",
                ErrorType.Conflict => "Conflict Error",
                _ => "Internal Server Error"
            };
        }
    }
}
