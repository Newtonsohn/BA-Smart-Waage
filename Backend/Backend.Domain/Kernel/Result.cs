using System.Diagnostics.CodeAnalysis;

namespace Backend.Domain.Kernel;

public class Result
{
    public Result(bool isSuccess, Error error)
    {
        if (isSuccess && error != Error.None ||
            !isSuccess && error == Error.None)
        {
            throw new ArgumentException("Invalid error", nameof(error));
        }

        IsSuccess = isSuccess;
        Error = error;
    }

    public bool IsSuccess { get; }

    public bool IsFailure => !IsSuccess;

    public Error Error { get; }

    public static Result Success() => new(true, Error.None);

    public static Result<TValue> Success<TValue>(TValue value) =>
        new(value, true, Error.None);

    public static Result Failure(Error error) => new(false, error);

    public static Result<TValue> Failure<TValue>(Error error) =>
        new(default, false, error);
}

public class Result<TValue> : Result
{
    private readonly TValue? _value;

    public Result(TValue? value, bool isSuccess, Error error)
        : base(isSuccess, error)
    {
        _value = value;
    }

    [NotNull]
    public TValue Value => IsSuccess
        ? _value!
        : throw new InvalidOperationException("The value of a failure result can't be accessed.");

    public static implicit operator Result<TValue>(TValue? value) =>
        value is not null ? Success(value) : Failure<TValue>(Error.NullValue);

    public static Result<TValue> ValidationFailure(Error error) =>
        new(default, false, error);
}

public static class ResultExtensions
{
    public static T Match<T>(
        this Result result,
        Func<T> onSuccess,
        Func<Error, T> onFailure)
    {
        return result.IsSuccess ? onSuccess() : onFailure(result.Error);
    }

    public static void Match<TResult>(
    this Result<TResult> result,
    Action<TResult> onSuccess,
    Action<Error> onFailure)
    {
        if (result.IsSuccess)
        {
            onSuccess(result.Value);
        }
        else
        {
            onFailure(result.Error);
        }
    }
    public static void Match(
    this Result result,
    Action onSuccess,
    Action<Error> onFailure)
    {
        if (result.IsSuccess)
        {
            onSuccess();
        }
        else
        {
            onFailure(result.Error);
        }
    }


    public static T Match<T, TResult>(
      this Result<TResult> result,
      Func<TResult, T> onSuccess,
      Func<Error, T> onFailure)
    {
        return result.IsSuccess ? onSuccess(result.Value) : onFailure(result.Error);
    }

    public static async Task<T> MatchAsync<T>(
    this Result result,
    Func<Task<T>> onSuccess,
    Func<Error, Task<T>> onFailure)
    {
        return result.IsSuccess
            ? await onSuccess()
            : await onFailure(result.Error);
    }

    public static async Task MatchAsync<TResult>(
        this Result<TResult> result,
        Func<TResult, Task> onSuccess,
        Func<Error, Task> onFailure)
    {
        if (result.IsSuccess)
        {
            await onSuccess(result.Value);
        }
        else
        {
            await onFailure(result.Error);
        }
    }

    public static async Task<T> MatchAsync<T, TResult>(
        this Result<TResult> result,
        Func<TResult, Task<T>> onSuccess,
        Func<Error, Task<T>> onFailure)
    {
        return result.IsSuccess
            ? await onSuccess(result.Value)
            : await onFailure(result.Error);
    }

    public static async Task<Result> MatchAsync<TResult>(
    this Result<TResult> result,
    Func<TResult, Task<Result>> onSuccess,
    Func<Error, Task<Result>> onFailure)
    {
        return result.IsSuccess
            ? await onSuccess(result.Value)
            : await onFailure(result.Error);
    }
}