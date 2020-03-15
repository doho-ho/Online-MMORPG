using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(Animator))]
public class BabaController : MonoBehaviour {

    public float idleTimeout = 5f;
    public float jumpSpeed = 10f;
    public float gravity = 20f;
    public bool IsGrounded = true;

    public float verticalSpeed;

    public Animator Baba_Animator;
    public CharacterController Baba_Controller;
    public float forwardSpeed = 0f;

    const float k_StickingGravityProportion = 0.3f;

    float idleTimer;

    // Parameters
    readonly int Baba_HashGrounded = Animator.StringToHash("Grounded");
    readonly int Baba_HashTimeoutToIdle = Animator.StringToHash("TimeoutToIdle");
    readonly int Baba_HashJump = Animator.StringToHash("JumpSpeed");
    private void Awake()
    {
        Baba_Animator = GetComponent<Animator>();
        Baba_Controller = GetComponent<CharacterController>();
    }
    // Use this for initialization

    // Update is called once per frame
    void FixedUpdate()
    {
        if (Input.GetButton("Jump") && IsGrounded)
        {
            verticalSpeed = jumpSpeed;
            IsGrounded = false;
        }

        if (IsGrounded && !Input.GetButton("Jump"))
        {
            idleTimer += Time.deltaTime;
            if(idleTimer >= idleTimeout)
            {
                idleTimer = 0;
                Baba_Animator.SetTrigger(Baba_HashTimeoutToIdle);
            }
        }
	}

    void OnAnimatorMove()
    {
        Vector3 movement;

        if (!IsGrounded)
            movement = forwardSpeed * transform.forward * Time.deltaTime;
        else
            movement = Baba_Animator.deltaPosition;

        movement += verticalSpeed * Vector3.up * Time.deltaTime;

        Baba_Controller.Move(movement);

        IsGrounded = Baba_Controller.isGrounded;
        if (!IsGrounded)
            Baba_Animator.SetFloat(Baba_HashJump, verticalSpeed);
        Baba_Animator.SetBool(Baba_HashGrounded, IsGrounded);
    }




}
